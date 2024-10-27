import librosa
from .manage_audio import preprocess_audio, strip_audio
import numpy as np
import torch
import torch.nn.functional as F
import pickle
import os.path

class sv_system():
    def __init__(self, model, lda_model=None, n_dims=40, feat_format='fbank'):
        self.filename = 'enrolled.pkl'
        if os.path.isfile(self.filename):
            with open(self.filename, 'rb') as f:
                self.speaker_models = pickle.load(f)
        else:
            self.speaker_models = dict()
        self.dct_filters = librosa.filters.dct(n_filters=n_dims, n_input=n_dims)
        self.model = model
        self.lda_model = lda_model
        self.n_dims = n_dims
        self.feat_format = feat_format

        # for test
#         self.speaker_models = self._random_speaker_model()

    def enrol(self, wav, spk_name):
        feat = self._wav2feat(wav)
        dvector = self._extract_dvector(feat).squeeze()
        if spk_name not in self.speaker_models:
            self.speaker_models[spk_name] = [dvector]
        else:
            self.speaker_models[spk_name] += [dvector]
        with open(self.filename, 'wb+') as f:
            pickle.dump(self.speaker_models, f, protocol=pickle.HIGHEST_PROTOCOL)

    def _extract_dvector(self, feat):
        """
            dvector: ndarray
        """
        if feat.dim() == 2:
            feat = feat.unsqueeze(0).unsqueeze(0)

        dvector = self.model.embed(feat).detach().cpu().numpy()

        # apply LDA if the lda_model was given
        if self.lda_model:
            dvector = self.lda_model.transform(dvector).astype(np.float32)


        return dvector

    def _wav2feat(self, wav):
        """
            extracting input feature from wav (mfcc, fbank)
        """
        wav_data = librosa.core.load(wav, sr=16000)[0]
        wav_data = strip_audio(wav_data, rms_ths=0.2)
        feat = preprocess_audio(wav_data, n_mels=self.n_dims,
                    dct_filters=self.dct_filters, in_feature=self.feat_format)

        return feat

    def _random_speaker_model(self):
        """
            random speaker model used for testing a sv_system
        """
        random_dvector = np.random.rand(4, self.model.embed_dim)
        random_speakers = ["a", "b", "c", "d"]

        random_speaker_models = dict.fromkeys(random_speakers)

        for i, key in enumerate(random_speaker_models.keys()):
            random_speaker_models[key] = [random_dvector[i]]*2

        return random_speaker_models

    def init_speaker_model(self):
        self.speaker_model = dict()

    def verify(self, wav):
        """
            verify a input wav and output a verification result
            and rank-1 identification
        """
        feat = self._wav2feat(wav)
        test_dvector = self._extract_dvector(feat)
        # order keep?
        # averaging all dvectors for each speaker
        avg_speaker_models = np.stack([np.mean(v, axis=0) for v in self.speaker_models.values()],
                                      axis=0)

        score = F.cosine_similarity(torch.from_numpy(avg_speaker_models).float(),
                                    torch.from_numpy(test_dvector).float(), dim=1)
        threshold = 0.65
        pred_speaker = list(self.speaker_models.keys())[torch.argmax(score)]

        if max(score) > threshold:
            print("Accepted as {} ({:.3f})".format(pred_speaker, max(score)))
            return True, pred_speaker, max(score)
        else:
            print("Reject ({:.3f})".format(max(score)))
            return False, 'noone', max(score)

def get_enrol_lst():
    filename = 'enrolled.pkl'
    if os.path.isfile(filename):
        with open(filename, 'rb') as f:
            speaker_models = pickle.load(f)
    else:
        speaker_models = dict()
    return speaker_models.keys()


def enrollment(data, name):
    from .ResNet34 import ResNet34, ResNet34_v1

    config = dict(
        loss="softmax",
        gpu_no=[0], no_cuda=True
    )

    model = ResNet34(config, inplanes=16, n_labels=1759)
    model.load_extractor("app/system/model_best.pth.tar")
    model.eval()
    lda_model = pickle.load(open("app/system/lda_model.pkl", "rb"))

    if not config['no_cuda']:
        model.cuda()

    test_sv_system = sv_system(model, lda_model)
    test_sv_system.enrol(data, name)

def verify(data):
    from .ResNet34 import ResNet34, ResNet34_v1

    config = dict(
        loss="softmax",
        gpu_no=[0], no_cuda=True
    )

    model = ResNet34(config, inplanes=16, n_labels=1759)
    model.load_extractor("app/system/model_best.pth.tar")
    model.eval()

    lda_model = pickle.load(open("app/system/lda_model.pkl", "rb"))

    if not config['no_cuda']:
        model.cuda()

    test_sv_system = sv_system(model, lda_model)
    return test_sv_system.verify(data)

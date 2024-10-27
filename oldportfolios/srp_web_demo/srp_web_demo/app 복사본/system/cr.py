import librosa
from .manage_audio import preprocess_audio
import numpy as np
import torch
import torch.nn.functional as F

class cr_system():
    def __init__(self, model, n_dims=40, feat_format='fbank'):
        self.classes = ['yes', 'no', 'up', 'down', 'left', 'right', 'on', 'off', 'stop', 'go']
        self.dct_filters = librosa.filters.dct(n_filters=n_dims, n_input=n_dims)
        self.model = model
        self.n_dims = n_dims
        self.feat_format = feat_format
        
    def _get_score(self, feat):
        """
            dvector: ndarray
        """
        if feat.dim() == 2:
            feat = feat.unsqueeze(0).unsqueeze(0)
        
        dvector = self.model.forward(feat).detach().cpu().numpy()
        
        return dvector
    
    def _wav2feat(self, wav):
        """
            extracting input feature from wav (mfcc, fbank)
        """
        wav_data = librosa.core.load(wav, sr=16000)[0]
        feat = preprocess_audio(wav_data, n_mels=self.n_dims, 
                    dct_filters=self.dct_filters, in_feature=self.feat_format)
        
        return feat

    def recog(self, wav):
        """
            verify a input wav and output a verification result
            and rank-1 identification
        """
        feat = self._wav2feat(wav)
        score = self._get_score(feat)
    
        pred_class = self.classes[np.argmax(score)]

        return pred_class
            
def recog(data):
    from .ResNet34 import ResNet34_cr
    
    config = dict(
        loss="softmax",
        gpu_no=[0], no_cuda=True
    )
    
    model = ResNet34_cr(config, inplanes=64, n_labels=10)
    model.load("app/system/cr_model_best.pth.tar")
    model.eval()
    
    if not config['no_cuda']:
        model.cuda()
    
    test_cr_system = cr_system(model)
    return test_cr_system.recog(data)        

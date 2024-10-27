import os
from django.core.files.base import File
from django.template import loader, RequestContext
from django.shortcuts import render, render_to_response, redirect
from django.http import HttpResponse, HttpResponseRedirect
from django.shortcuts import render
import pandas as pd
from django.core.files.storage import default_storage
from django.core.files.base import ContentFile

from .forms import EnrollForm, TestForm
from .system import sv, cr
import librosa.core
import json
import pickle

# Create your views here.
def Index(request):
    return render(request, 'demo/home.html')

def Enrollment(request):
    if request.method == 'POST':
        name = request.POST['name']
        upload_file = request.FILES['data_blob']
        if upload_file:
            path = default_storage.save('222.wav', ContentFile(upload_file.read()))
            sv.enrollment('media/'+path, name)

        return HttpResponse('enrolled')

    enrol_lst = sv.get_enrol_lst()
    return render(request, 'demo/enrollment.html', {'enrol_lst': enrol_lst})

def Test(request):
    if request.method == 'POST':
        upload_file = request.FILES['data_blob']
        if upload_file:
            name = 'test'
            file_name = name + '.wav'
            path = default_storage.save(file_name, ContentFile(upload_file.read()))
            accept, speaker, score = sv.verify('media/'+path)
            command = cr.recog('media/'+path)
            context = {
                'accept': accept,
                'speaker': speaker,
                'command': command,
            }
            with open('test_wav.in', 'wb') as f:
                pickle.dump(context, f, protocol=pickle.HIGHEST_PROTOCOL)

            return HttpResponse('done')

    return render(request, 'demo/test.html')

def Result(request):
    context = {}
    with open('test_wav.in', 'rb') as f:
        context = pickle.load(f)

    return render(request, 'demo/result.html', context)

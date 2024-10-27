from django import forms

class EnrollForm(forms.Form):
    name = forms.CharField(label='name', widget=forms.TextInput)
    record = forms.CharField(label='record', widget=forms.TextInput)

class TestForm(forms.Form):
    record = forms.CharField(label='record', widget=forms.TextInput)

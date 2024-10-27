from django.conf import settings
from django.urls import path
from django.contrib.staticfiles.urls import static, staticfiles_urlpatterns

from .views import (
    Index,
    Enrollment,
    Test,
    Result,
)

urlpatterns = [
    path('', Index, name='home'),
    path('enrollment/', Enrollment, name='enrollment'),
    path('test/', Test, name='test'),
    path('result/', Result, name='result'),
]

if settings.DEBUG:
    urlpatterns += static(
        settings.MEDIA_URL,
        document_root=settings.MEDIA_ROOT)
    urlpatterns += staticfiles_urlpatterns()

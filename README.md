microsoft AKS Store Demo 
(https://github.com/Azure-Samples/aks-store-demo?tab=readme-ov-file)
를 기반으로 한

트릭컬 서버 프로그래머 포트 폴리오입니다.

Docker
Azure
Kubernetes
RabbitMQ
MongoDB
helm
를 사용해보았습니다.

C++ 과 C#에 관해

기본적인 문법은 같으나 C++ 개발 이력이 길어서 특히 new 를 통해서 새로운 인스턴스를 다루는 것에는 매우 익숙합니다.
C++을 주로 배우고 C#은 따로 배우지 않았으나 C#처럼 객체 지향 언어인 Python 개발 경력을 통해
객체 지향 언어는 기본적으로 다룰 줄 압니다.
C#에서 지역 함수에서는 var을 통해서 가독성 및 생산성을 향상하는 것,
IEnumerable<var> 의 벡터와 유사한 표현은 다양한 STL 경험을 통해 익숙합니다.
C#의 강력한 기능이 상속을 받지 않고 Method(C, C++ 의 함수)를 추가할 수 있는 강력한 기능인 Extension Method,
을 충분히 다룰 줄 압니다.

Linq와 Reflection은 현재 정확하게 코드로 생산할 정도가 되지는 않지만,
Linq의 from, where, orderby, select, groupby의 메소드의 동작 방식과 기본 개념은 알고 있습니다.

aks 서비스에 VCPU를 할당 받지 못하여 실제 AKS를 등록하진 못했으나 개인 계정 azure registry(Region : koreacentral, https://portfolio0126.azurecr.io)에 등록하여 helm 예제까지는 실행하였습니다.


➜  mychart git:(main) az acr manifest list-metadata \
  --registry portfolio1026 \
  --name helm/mychart
Command group 'acr manifest' is in preview and under development. Reference and support levels: https://aka.ms/CLI_refstatus
[
  {
    "changeableAttributes": {
      "deleteEnabled": true,
      "listEnabled": true,
      "readEnabled": true,
      "writeEnabled": true
    },
    "configMediaType": "application/vnd.cncf.helm.config.v1+json",
    "createdTime": "2024-10-27T08:10:10.1480953Z",
    "digest": "sha256:1b47d042b28c82fd902e4e68f4a6ef0401411ebd3b1a01da1443d6a89fcf7a33",
    "imageSize": 2068,
    "lastUpdateTime": "2024-10-27T08:10:10.1480953Z",
    "mediaType": "application/vnd.oci.image.manifest.v1+json",
    "tags": [
      "0.1.0"
    ]
  }
]
➜  mychart git:(main) helm install myhelmtest oci://portfolio0126.azurecr.io/helm/mychart --version 0.1.0
Error: INSTALLATION FAILED: failed to do request: Head "https://portfolio0126.azurecr.io/v2/helm/mychart/manifests/0.1.0": dial tcp: lookup portfolio0126.azurecr.io: no such host
➜  mychart git:(main) helm install myhelmtest oci://portfolio1026.azurecr.io/helm/mychart --version 0.1.0 
Pulled: portfolio1026.azurecr.io/helm/mychart:0.1.0
Digest: sha256:1b47d042b28c82fd902e4e68f4a6ef0401411ebd3b1a01da1443d6a89fcf7a33
Error: INSTALLATION FAILED: Kubernetes cluster unreachable: Get "http://localhost:8080/version": dial tcp [::1]:8080: connect: connection refused
➜  mychart git:(main) helm get manifest myhelmtest                                                       
Error: Kubernetes cluster unreachable: Get "http://localhost:8080/version": dial tcp [::1]:8080: connect: connection refused

CLI 상 확인 가능한 현재 진행 상황입니다.

VCPU AKS 유료 서비스를 신청하지 못하여

Redis 와 ArgoCD, PostgreSQL를 직접 사용해보진 못하였으나

https://devocean.sk.com/blog/techBoardDetail.do?ID=165961&boardType=techBlog&searchData=&searchDataMain=CLOUD_VIRTUAL&page=&subIndex=&searchText=%23Kubernetes&techType=BLOG&searchDataSub=

https://devocean.sk.com/blog/techBoardDetail.do?ID=164752&boardType=techBlog

https://www.sktenterprise.com/bizInsight/blogDetail/dev/11242

블로그 등을 통하여 SaaS 개념을 학습하였습니다.

이후 개인 Azure계정에서 SaaS로 접근이 가능한 Redis와 PostgreSQL를 확인하였고, ArgoCD는 helm예제와 연계하여 학습하였습니다.


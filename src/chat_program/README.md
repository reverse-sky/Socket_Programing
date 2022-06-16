# 서버가 존재할때 4명의 클라이언트가 접속해서 채팅하는 시스템 

## 환경 
> kali_linux
> vscode로 구현

## 구현 방법
>1. 서버와 client는 AF_INET을 이용하여 연결이 된다. 
>2. client는 AF_UNET을 이용하여 모니터와 키보드가 서로 통신

## 초기 시작
> 초기 시작은 다음과 같은 9개의 터미널로 시작을 한다. 
<img src ="https://user-images.githubusercontent.com/45085563/174059833-7c34f990-38ee-4c68-bfc1-9b048019866a.png"  alt="img1" height="500" width="600">   

## 서버 시작 
>서버 터미널을 구동한다.
<img src ="https://user-images.githubusercontent.com/45085563/174060448-6438b77d-8954-42c4-a04d-32087a39d31f.png"  alt="img2" height="500" width="600">   

## 클라이언트가 서버에 접속 
>
....

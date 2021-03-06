[![GitHub version](https://badge.fury.io/gh/joeuninsrnd%2Fbxr_plover.svg)](https://badge.fury.io/gh/joeuninsrnd%2Fbxr_plover)

## 개요
 bxr_plover(민감정보 생명주기 관리프로그램)는 최근 정부에서 진행되는 개방형 OS 도입 및 확산 프로젝트의 한 부분으로 진행되는 연구 개발로써 개방형 OS 환경에서의 민감정보 검출 및 유출을 방지하기 위한 방향으로 진행할 것이며, RabbitMQ를 사용하여 Server와 Client 통신을 하고 MariaDB, Tomcat을 사용할 예정이다. 현재 간단한 UI를 통한 Server/Client 구조로 txt파일 형태의 민감정보(주민등록번호) 검출이 가능하며 hwp, pdf 등과 같은 다양한 문서포맷을 검출하는 모듈을 개발중이다. 이후에 여권번호, 운전면허번호 등과 같은 다양한 민감정보를 검출하도록 확장할 계획이며, 관리자 페이지와 아이콘도 같이 개발 및 디자인 중이다.<br><br>
**&#35;&#35;프로세스 구상도 및 사용 도구&#35;&#35;**<br>Client <=> Server <=> Web <=> Administrator<br><br>
*&#35;Client&#35;*<br>Debian 10 <br>GTK+ 3.24.10 <br>RabbitMQ 3.7.17 <br><br>
*&#35;Server&#35;*<br>CentOS 7.6.1810 <br>RabbitMQ 3.7.17 <br>MariaDB 10.4.7 <br><br>
*&#35;Web&#35;*<br> Tomcat <br>Web/WAS 9.0.21 <br><br>
*&#35;Administrator&#35;*<br>Web Console
설명...아키텍처, 이미지

## 사용방법
설치,이용방법...

## 개발방법
1. download
1. build
1. run

## 라이선스
This project is licensed under the GPL v3.0 License.<br>
https://github.com/joeuninsrnd/bxr_plover/blob/master/LICENSE.md<br>
https://github.com/joeuninsrnd/bxr_plover/wiki/about-LICENSE

## 참여방법
https://joeunins.slack.com<br>
https://joeuninsrnd.github.io<br>
https://github.com/joeuninsrnd/bxr_plover/blob/master/CONTRIBUTING.md<br>
old: https://github.com/joeunins/bxr_plover<br>

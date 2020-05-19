FROM mcr.microsoft.com/vscode/devcontainers/base:0-debian-10

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
   && apt-get -y install --no-install-recommends build-essential texlive-latex-extra
RUN apt-get -y install clang
ENV DEBIAN_FRONTEND=dialog

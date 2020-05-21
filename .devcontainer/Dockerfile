FROM trzeci/emscripten:1.39.10-upstream

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
   && apt-get -y install --no-install-recommends build-essential texlive-latex-extra
RUN apt-get -y install clang

COPY ./*.sh /scripts/
RUN "bash" "/scripts/build.sh"

RUN apt-get -y install entr

ENV DEBIAN_FRONTEND=dialog

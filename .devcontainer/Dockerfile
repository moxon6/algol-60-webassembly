FROM trzeci/emscripten:1.39.10-upstream

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
   && apt-get -y install --no-install-recommends build-essential texlive-latex-extra
RUN apt-get -y install clang

COPY ./*.sh /scripts/
RUN "bash" "/scripts/build.sh"

RUN apt-get -y install entr locales

RUN sed -i -e 's/# en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/' /etc/locale.gen && \
    dpkg-reconfigure --frontend=noninteractive locales && \
    update-locale LANG=en_US.UTF-8

RUN sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmybash/oh-my-bash/master/tools/install.sh)" 
RUN sed -i "s/OSH_THEME=.*/OSH_THEME=\"sirup\"/g" ~/.bashrc

RUN apt-get -y install moreutils

ENV DEBIAN_FRONTEND=dialog

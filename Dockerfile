FROM ruby:3.3-bookworm
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    lldb-14 \
    liblldb-14-dev \
    libffi-dev \
    gcc \
    g++ \
    make \
    && rm -rf /var/lib/apt/lists/*

ENV PATH="/usr/lib/llvm-14/bin:$PATH"
ENV LLDB_DEBUGSERVER_PATH="/usr/lib/llvm-14/bin/lldb-server"
WORKDIR /app
COPY Gemfile Gemfile.lock lldb.gemspec ./
COPY lib/lldb/version.rb ./lib/lldb/
RUN bundle install
COPY . .
RUN bundle exec rake compile
RUN rm -f spec/fixtures/simple spec/fixtures/loop && \
    gcc -g -O0 -o spec/fixtures/simple spec/fixtures/simple.c && \
    gcc -g -O0 -o spec/fixtures/loop spec/fixtures/loop.c
CMD ["bundle", "exec", "rspec", "--format", "documentation"]

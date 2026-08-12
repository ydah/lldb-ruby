# frozen_string_literal: true

source 'https://rubygems.org'

gemspec

gem 'rake', '~> 13.0'
gem 'rspec', '~> 3.0'

# RBS and Steep dropped Ruby 3.0 support. They are development-only tools,
# so keep them available on supported Rubies without preventing the minimum
# runtime compatibility job from resolving its test dependencies.
if Gem::Version.new(RUBY_VERSION) >= Gem::Version.new('3.1')
  gem 'rbs', '~> 3.0'
  gem 'rbs-inline', '~> 0.12'
  gem 'steep', '~> 1.9'
end

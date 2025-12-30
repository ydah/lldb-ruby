# frozen_string_literal: true

require_relative 'lib/lldb/version'

Gem::Specification.new do |spec|
  spec.name          = 'lldb'
  spec.version       = LLDB::VERSION
  spec.authors       = ['Yudai Takada']
  spec.email         = ['t.yudai92@gmail.com']

  spec.summary       = 'Ruby bindings for LLDB debugger'
  spec.description   = 'Access LLDB debugger functionality from Ruby via FFI'
  spec.homepage      = 'https://github.com/ydah/lldb-ruby'
  spec.licenses      = ['MIT', 'Apache-2.0']
  spec.required_ruby_version = '>= 3.0.0'

  spec.metadata['source_code_uri'] = spec.homepage
  spec.metadata['changelog_uri'] = "#{spec.homepage}/blob/main/CHANGELOG.md"
  spec.metadata['rubygems_mfa_required'] = 'true'

  spec.files = Dir.chdir(__dir__) do
    `git ls-files -z`.split("\x0").reject do |f|
      (File.expand_path(f) == __FILE__) ||
        f.start_with?(*%w[bin/ test/ spec/ features/ .git .github appveyor Gemfile Dockerfile])
    end
  end

  spec.files += Dir['lib/**/*.rb', 'ext/**/*']
  spec.extensions = ['ext/lldb/extconf.rb']
  spec.require_paths = ['lib']

  spec.add_dependency 'ffi', '~> 1.15'
end

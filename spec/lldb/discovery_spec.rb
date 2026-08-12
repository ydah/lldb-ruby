# frozen_string_literal: true

require 'tmpdir'
require_relative '../../ext/lldb/discovery'

RSpec.describe LLDB::BuildDiscovery do
  describe '.llvm_config_paths' do
    it 'prefers unversioned llvm-config and then sorts versioned binaries' do
      Dir.mktmpdir('llvm-config') do |directory|
        %w[llvm-config-16 llvm-config-14 llvm-config].each do |name|
          path = File.join(directory, name)
          File.write(path, '')
          File.chmod(0o755, path)
        end

        expect(described_class.llvm_config_paths(directory).map { |path| File.basename(path) }).to eq(
          %w[llvm-config llvm-config-16 llvm-config-14]
        )
      end
    end
  end

  describe '.candidates' do
    it 'puts explicit paths before discovered paths' do
      candidates = described_class.candidates(
        { include: '/explicit/include', lib: '/explicit/lib' },
        path: ''
      )

      expect(candidates.first).to have_attributes(
        include_dir: '/explicit/include',
        lib_dir: '/explicit/lib'
      )
    end
  end
end

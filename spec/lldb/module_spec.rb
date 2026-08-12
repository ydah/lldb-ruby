# frozen_string_literal: true

RSpec.describe LLDB::Module do
  let(:debugger) { LLDB::Debugger.create }
  let(:executable) { compile_fixture('simple') }
  let(:target) { debugger.create_target(executable) }
  let(:mod) { target.module_at_index(0) }

  describe '#valid?' do
    it 'returns true for a valid module' do
      expect(mod).to be_valid
    end
  end

  describe '#file_path' do
    it 'returns the path to the module' do
      expect(mod.file_path).to eq(executable)
    end

    it 'exposes the module file as a FileSpec' do
      expect(mod.file).to be_a(LLDB::FileSpec)
      expect(mod.file.path).to eq(executable)
    end
  end

  describe '#num_symbols' do
    it 'returns the number of symbols' do
      expect(mod.num_symbols).to be >= 0
    end

    it 'returns structured symbols' do
      skip 'No symbols found' if mod.num_symbols.zero?

      symbol = mod.symbol_at_index(0)
      expect(symbol).to be_a(LLDB::Symbol)
      expect(symbol).to be_valid
      expect(symbol.name || symbol.display_name).to be_a(String)
    end
  end

  describe '#to_s' do
    it 'returns the file path' do
      expect(mod.to_s).to eq(executable)
    end
  end
end

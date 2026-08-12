# frozen_string_literal: true

RSpec.describe LLDB::FileSpec do
  let(:executable) { compile_fixture('simple') }

  describe '#valid?' do
    it 'represents an existing path' do
      file_spec = described_class.new(executable, resolve: true)

      expect(file_spec).to be_valid
      expect(file_spec).to be_exists
      expect(file_spec.path).to eq(executable)
      expect(file_spec.filename).to eq(File.basename(executable))
      expect(file_spec.directory).to eq(File.dirname(executable))
    end

    it 'represents a default invalid file spec' do
      expect(described_class.new).not_to be_valid
      expect(described_class.new.exists?).to be false
    end
  end

  describe '#filename=' do
    it 'updates the filename and directory independently' do
      file_spec = described_class.new(executable, resolve: true)

      file_spec.filename = 'renamed'
      expect(file_spec.filename).to eq('renamed')

      file_spec.directory = File.dirname(executable)
      expect(file_spec.directory).to eq(File.dirname(executable))
    end
  end

  describe 'path validation' do
    it 'rejects embedded NUL bytes' do
      expect do
        described_class.new("invalid\0path")
      end.to raise_error(ArgumentError)
    end
  end
end

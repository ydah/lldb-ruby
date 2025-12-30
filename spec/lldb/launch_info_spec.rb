# frozen_string_literal: true

RSpec.describe LLDB::LaunchInfo do
  describe '#initialize' do
    it 'creates a launch info without arguments' do
      info = LLDB::LaunchInfo.new
      expect(info).not_to be_nil
    end

    it 'creates a launch info with arguments' do
      info = LLDB::LaunchInfo.new(%w[arg1 arg2])
      expect(info).not_to be_nil
    end
  end

  describe '#working_directory=' do
    it 'sets the working directory' do
      info = LLDB::LaunchInfo.new
      expect { info.working_directory = '/tmp' }.not_to raise_error
    end
  end

  describe '#set_environment' do
    it 'sets environment variables' do
      info = LLDB::LaunchInfo.new
      expect do
        info.set_environment({ 'FOO' => 'bar', 'BAZ' => 'qux' })
      end.not_to raise_error
    end

    it 'can append to existing environment' do
      info = LLDB::LaunchInfo.new
      expect do
        info.set_environment({ 'FOO' => 'bar' }, append: true)
      end.not_to raise_error
    end
  end
end

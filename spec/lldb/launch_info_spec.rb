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

    it 'distinguishes nil and an empty replacement environment' do
      info = LLDB::LaunchInfo.new
      expect { info.set_environment(nil) }.not_to raise_error
      expect { info.set_environment({}, append: false) }.not_to raise_error
    end

    it 'rejects invalid native string content' do
      info = LLDB::LaunchInfo.new
      expect { info.set_environment({ "BAD=KEY" => 'value' }) }
        .to raise_error(ArgumentError, /environment keys/)
      expect { info.set_environment({ 'BAD' => "value\0" }) }
        .to raise_error(ArgumentError, /NUL/)
    end
  end
end

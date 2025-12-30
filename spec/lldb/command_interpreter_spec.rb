# frozen_string_literal: true

RSpec.describe LLDB::CommandInterpreter do
  let(:debugger) { LLDB::Debugger.create }
  let(:interpreter) { debugger.command_interpreter }

  describe '#valid?' do
    it 'returns true for a valid interpreter' do
      expect(interpreter).to be_valid
    end
  end

  describe '#handle_command' do
    it 'executes a command and returns a result' do
      result = interpreter.handle_command('help')
      expect(result).to be_a(LLDB::CommandReturnObject)
      expect(result).to be_valid
    end

    it 'returns succeeded? for successful commands' do
      result = interpreter.handle_command('help')
      expect(result.succeeded?).to be true
    end

    it 'returns output for commands with output' do
      result = interpreter.handle_command('version')
      expect(result.output).to be_a(String)
    end
  end

  describe '#command_exists?' do
    it 'returns true for existing commands' do
      expect(interpreter.command_exists?('help')).to be true
      expect(interpreter.command_exists?('version')).to be true
    end

    it 'returns false for non-existing commands' do
      expect(interpreter.command_exists?('nonexistent_command_xyz')).to be false
    end
  end

  describe '#alias_exists?' do
    it 'returns true for existing aliases' do
      expect(interpreter.alias_exists?('q')).to be true
    end

    it 'returns false for non-existing aliases' do
      expect(interpreter.alias_exists?('nonexistent_alias_xyz')).to be false
    end
  end
end

RSpec.describe LLDB::CommandReturnObject do
  describe '.create' do
    it 'creates a valid command return object' do
      obj = LLDB::CommandReturnObject.create
      expect(obj).to be_a(LLDB::CommandReturnObject)
      expect(obj).to be_valid
    end
  end

  describe '#clear' do
    it 'clears the object' do
      debugger = LLDB::Debugger.create
      interpreter = debugger.command_interpreter
      result = interpreter.handle_command('help')

      result.clear
      expect(result.output).to be_empty
    end
  end
end

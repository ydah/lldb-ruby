# frozen_string_literal: true

RSpec.describe LLDB::Debugger do
  describe '.create' do
    it 'creates a valid debugger' do
      debugger = LLDB::Debugger.create
      expect(debugger).to be_valid
    end
  end

  describe '#create_target' do
    let(:debugger) { LLDB::Debugger.create }
    let(:executable) { compile_fixture('simple') }

    it 'creates a target from an executable' do
      target = debugger.create_target(executable)
      expect(target).to be_a(LLDB::Target)
      expect(target).to be_valid
    end

    it 'raises an error for non-existent file' do
      expect do
        debugger.create_target('/nonexistent/path/to/executable')
      end.to raise_error(LLDB::LLDBError)
    end
  end

  describe '#create_target_simple' do
    let(:debugger) { LLDB::Debugger.create }
    let(:executable) { compile_fixture('simple') }

    it 'creates a target from an executable' do
      target = debugger.create_target_simple(executable)
      expect(target).to be_a(LLDB::Target)
      expect(target).to be_valid
    end
  end

  describe '#num_targets' do
    let(:debugger) { LLDB::Debugger.create }
    let(:executable) { compile_fixture('simple') }

    it 'returns 0 for a new debugger' do
      expect(debugger.num_targets).to eq(0)
    end

    it 'returns the number of targets after creation' do
      debugger.create_target(executable)
      expect(debugger.num_targets).to eq(1)
    end
  end

  describe '#targets' do
    let(:debugger) { LLDB::Debugger.create }
    let(:executable) { compile_fixture('simple') }

    it 'returns an empty array for a new debugger' do
      expect(debugger.targets).to eq([])
    end

    it 'returns all targets' do
      debugger.create_target(executable)
      targets = debugger.targets
      expect(targets.length).to eq(1)
      expect(targets.first).to be_valid
    end
  end

  describe '#async' do
    let(:debugger) { LLDB::Debugger.create }

    it 'can get and set async mode' do
      debugger.async = true
      expect(debugger.async?).to be true

      debugger.async = false
      expect(debugger.async?).to be false
    end
  end

  describe '.version_string' do
    it 'returns the LLDB version string' do
      version = LLDB::Debugger.version_string
      expect(version).to be_a(String)
      expect(version).not_to be_empty
    end
  end

  describe '#command_interpreter' do
    let(:debugger) { LLDB::Debugger.create }

    it 'returns a valid command interpreter' do
      interpreter = debugger.command_interpreter
      expect(interpreter).to be_a(LLDB::CommandInterpreter)
      expect(interpreter).to be_valid
    end
  end

  describe '#handle_command' do
    let(:debugger) { LLDB::Debugger.create }

    it 'executes an LLDB command' do
      # Use 'version' instead of 'settings show' to avoid massive output on CI
      expect { debugger.handle_command('version') }.not_to raise_error
    end
  end

  describe '#selected_target=' do
    let(:debugger) { LLDB::Debugger.create }
    let(:executable) { compile_fixture('simple') }

    it 'sets the selected target' do
      target = debugger.create_target(executable)
      debugger.selected_target = target
      expect(debugger.selected_target).to be_valid
    end
  end

  describe '#delete_target' do
    let(:debugger) { LLDB::Debugger.create }
    let(:executable) { compile_fixture('simple') }

    it 'deletes a target' do
      target = debugger.create_target(executable)
      expect(debugger.num_targets).to eq(1)

      result = debugger.delete_target(target)
      expect(result).to be true
      expect(debugger.num_targets).to eq(0)
    end
  end
end

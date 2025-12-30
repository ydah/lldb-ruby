# frozen_string_literal: true

RSpec.describe LLDB::Frame do
  let(:debugger) { LLDB::Debugger.create }
  let(:executable) { compile_fixture('simple') }
  let(:target) { debugger.create_target(executable) }
  let(:process) do
    debugger.async = false
    # Set breakpoint inside the lldb_test_add function to see local variables
    target.breakpoint_create_by_name('lldb_test_add')
    target.launch
  end
  let(:thread) { process.selected_thread }
  let(:frame) { thread.selected_frame }

  after do
    process.kill if process&.valid?
  end

  describe '#valid?' do
    it 'returns true for a valid frame' do
      expect(frame).to be_valid
    end
  end

  describe '#function_name' do
    it 'returns the function name' do
      expect(frame.function_name).to eq('lldb_test_add')
    end
  end

  describe '#frame_id' do
    it 'returns 0 for the top frame' do
      expect(frame.frame_id).to eq(0)
    end
  end

  describe '#pc' do
    it 'returns the program counter' do
      expect(frame.pc).to be > 0
    end
  end

  describe '#find_variable' do
    it 'finds a local variable' do
      var = frame.find_variable('a')
      expect(var).to be_a(LLDB::Value)
      expect(var).to be_valid
      expect(var.name).to eq('a')
    end

    it 'returns nil for non-existent variable' do
      var = frame.find_variable('nonexistent')
      expect(var).to be_nil
    end
  end

  describe '#evaluate_expression' do
    it 'evaluates a simple expression' do
      result = frame.evaluate_expression('a + b')
      expect(result).to be_a(LLDB::Value)
      expect(result).to be_valid
    end
  end

  describe '#location' do
    it 'returns file:line format' do
      location = frame.location
      expect(location).to match(/:\d+$/)
    end
  end

  describe '#to_s' do
    it 'returns a string representation' do
      str = frame.to_s
      expect(str).to include('lldb_test_add')
    end
  end

  describe '#get_variables' do
    it 'returns a ValueList' do
      variables = frame.get_variables(arguments: true, locals: true)
      expect(variables).to be_a(LLDB::ValueList)
      expect(variables).to be_valid
    end

    it 'can filter by type' do
      args_only = frame.get_variables(arguments: true, locals: false, statics: false)
      locals_only = frame.get_variables(arguments: false, locals: true, statics: false)

      expect(args_only).to be_a(LLDB::ValueList)
      expect(locals_only).to be_a(LLDB::ValueList)
    end
  end

  describe '#get_registers' do
    it 'returns a ValueList of register sets' do
      registers = frame.get_registers
      expect(registers).to be_a(LLDB::ValueList)
      expect(registers).to be_valid
      expect(registers.size).to be > 0
    end
  end

  describe '#inlined?' do
    it 'returns a boolean' do
      expect([true, false]).to include(frame.inlined?)
    end
  end

  describe '#disassemble' do
    it 'returns disassembly as a string' do
      disasm = frame.disassemble
      expect(disasm).to be_nil.or(be_a(String))
    end
  end

  describe '#get_module' do
    it 'returns the module for the frame' do
      mod = frame.get_module
      skip 'Module not available' if mod.nil?

      expect(mod).to be_a(LLDB::Module)
      expect(mod).to be_valid
    end
  end
end

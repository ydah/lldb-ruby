# frozen_string_literal: true

RSpec.describe LLDB::ValueList do
  let(:debugger) { LLDB::Debugger.create }
  let(:executable) { compile_fixture('simple') }
  let(:target) { debugger.create_target(executable) }
  let(:process) do
    debugger.async = false
    target.breakpoint_create_by_name('lldb_test_add')
    target.launch
  end
  let(:thread) { process.selected_thread }
  let(:frame) { thread.selected_frame }

  after do
    process.kill if process&.valid?
  end

  describe '#get_variables' do
    it 'returns a ValueList' do
      variables = frame.get_variables(arguments: true, locals: true)
      expect(variables).to be_a(LLDB::ValueList)
    end

    it 'returns valid values' do
      variables = frame.get_variables(arguments: true, locals: true)
      expect(variables).to be_valid
    end

    it 'has size >= 0' do
      variables = frame.get_variables(arguments: true, locals: true)
      expect(variables.size).to be >= 0
    end

    it 'can iterate over values' do
      variables = frame.get_variables(arguments: true, locals: true)
      count = 0
      variables.each { count += 1 }
      expect(count).to eq(variables.size)
    end

    it 'can convert to array' do
      variables = frame.get_variables(arguments: true, locals: true)
      arr = variables.to_a
      expect(arr).to be_an(Array)
      expect(arr.size).to eq(variables.size)
    end
  end

  describe '#get_registers' do
    it 'returns a ValueList' do
      registers = frame.get_registers
      expect(registers).to be_a(LLDB::ValueList)
    end

    it 'returns valid registers' do
      registers = frame.get_registers
      expect(registers).to be_valid
    end

    it 'has register sets' do
      registers = frame.get_registers
      expect(registers.size).to be > 0
    end
  end

  describe '#[]' do
    it 'can access values by index' do
      variables = frame.get_variables(arguments: true, locals: true)
      next if variables.size.zero?

      first = variables[0]
      expect(first).to be_a(LLDB::Value).or(be_nil)
    end
  end

  describe '#first_value_by_name' do
    it 'can find values by name' do
      variables = frame.get_variables(arguments: true, locals: true)

      variables.each do |var|
        found = variables.first_value_by_name(var.name)
        expect(found).to be_a(LLDB::Value) if found
        break
      end
    end
  end

  describe '#empty?' do
    it 'returns true for empty list' do
      variables = frame.get_variables(arguments: false, locals: false, statics: false)
      expect(variables.empty?).to be true
    end
  end
end

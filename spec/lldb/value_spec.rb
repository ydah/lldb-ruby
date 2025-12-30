# frozen_string_literal: true

RSpec.describe LLDB::Value do
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
  let(:value) { frame.find_variable('a') }

  after do
    process.kill if process&.valid?
  end

  describe '#valid?' do
    it 'returns true for a valid value' do
      expect(value).to be_valid
    end
  end

  describe '#name' do
    it 'returns the variable name' do
      expect(value.name).to eq('a')
    end
  end

  describe '#type_name' do
    it 'returns the type name' do
      expect(value.type_name).to eq('int')
    end
  end

  describe '#value' do
    it 'returns the value as string' do
      expect(value.value).to eq('10')
    end
  end

  describe '#value_as_signed' do
    it 'returns the value as signed integer' do
      expect(value.value_as_signed).to eq(10)
    end
  end

  describe '#to_i' do
    it 'is an alias for value_as_signed' do
      expect(value.to_i).to eq(10)
    end
  end

  describe '#byte_size' do
    it 'returns the size of the value' do
      expect(value.byte_size).to eq(4) # int is 4 bytes
    end
  end

  describe '#to_s' do
    it 'returns a string representation' do
      str = value.to_s
      expect(str).to include('a')
      expect(str).to include('10')
    end
  end

  describe '#inspect' do
    it 'returns a detailed representation' do
      str = value.inspect
      expect(str).to include('LLDB::Value')
      expect(str).to include('a')
      expect(str).to include('int')
    end
  end

  describe 'Enumerable' do
    # Test with a struct or array to check enumerable
    let(:main_frame) do
      # Continue to main to test variables there
      target.delete_breakpoint(1)
      target.breakpoint_create_by_name('main')
      process.continue
      process.selected_thread.selected_frame
    end

    it 'is enumerable' do
      expect(value).to respond_to(:each)
      expect(value).to respond_to(:map)
    end
  end

  describe '#type' do
    it 'returns a Type object' do
      type = value.type
      skip 'Type not available' if type.nil?

      expect(type).to be_a(LLDB::Type)
      expect(type).to be_valid
    end
  end

  describe '#load_address' do
    it 'returns the load address' do
      address = value.load_address
      expect(address).to be_a(Integer)
    end
  end

  describe '#value_type' do
    it 'returns a valid value type' do
      vt = value.value_type
      expect(vt).to be_a(Integer)
    end
  end

  describe '#value_type_name' do
    it 'returns a string for the value type' do
      name = value.value_type_name
      expect(name).to be_a(String)
    end
  end

  describe '#expression_path' do
    it 'returns the expression path' do
      path = value.expression_path
      expect(path).to be_nil.or(be_a(String))
    end
  end

  describe '#pointer_type?' do
    it 'returns false for an int' do
      expect(value.pointer_type?).to be false
    end
  end
end

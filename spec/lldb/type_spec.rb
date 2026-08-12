# frozen_string_literal: true

RSpec.describe LLDB::Type do
  let(:debugger) { LLDB::Debugger.create }
  let(:executable) { compile_fixture('type_members') }
  let(:target) { debugger.create_target(executable) }
  let(:process) do
    debugger.async = false
    target.breakpoint_create_by_name('lldb_test_type')
    target.launch
  end
  let(:thread) { process.selected_thread }
  let(:frame) { thread.selected_frame }
  let(:value) { frame.find_variable('value') }
  let(:type) { value&.type }

  after do
    process.kill if process&.valid?
  end

  describe '#valid?' do
    it 'returns true for a valid type' do
      skip 'Value or type not found' if type.nil?
      expect(type).to be_valid
    end
  end

  describe '#name' do
    it 'returns the type name' do
      skip 'Value or type not found' if type.nil?
      expect(type.name).to be_a(String)
    end
  end

  describe '#byte_size' do
    it 'returns the byte size of the type' do
      skip 'Value or type not found' if type.nil?
      expect(type.byte_size).to be >= 0
    end
  end

  describe '#basic_type' do
    it 'keeps basic type values stable across LLDB enum insertions' do
      skip 'Value or type not found' if type.nil?

      first_field_type = type.field_at_index(0)&.type
      skip 'First field type not found' if first_field_type.nil?

      expect(first_field_type.basic_type).to eq(LLDB::BasicType::INT)
    end
  end

  describe '#pointer_type?' do
    it 'returns a boolean' do
      skip 'Value or type not found' if type.nil?
      expect([true, false]).to include(type.pointer_type?)
    end
  end

  describe '#to_s' do
    it 'returns a string representation' do
      skip 'Value or type not found' if type.nil?
      expect(type.to_s).to be_a(String)
    end
  end

  describe '#field_at_index' do
    it 'exposes field metadata and nested types' do
      skip 'Value or type not found' if type.nil?

      expect(type.num_fields).to be >= 3
      field = type.field_at_index(0)
      expect(field).to be_a(LLDB::TypeMember)
      expect(field).to be_valid
      expect(field.name).to eq('first')
      expect(field.type).to be_a(LLDB::Type)
      expect(field.offset_in_bytes).to eq(0)

      bitfield = type.field_at_index(1)
      expect(bitfield.name).to eq('flags')
      expect(bitfield.bitfield_size_in_bits).to eq(3)
    end

    it 'returns nil for an out-of-range field' do
      skip 'Value or type not found' if type.nil?

      expect(type.field_at_index(type.num_fields)).to be_nil
    end
  end

  describe '#base class accessors' do
    it 'return nil when the type has no base classes' do
      skip 'Value or type not found' if type.nil?

      expect(type.direct_base_class_at_index(0)).to be_nil
      expect(type.virtual_base_class_at_index(0)).to be_nil
    end
  end
end

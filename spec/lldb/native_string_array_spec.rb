# frozen_string_literal: true

RSpec.describe LLDB::NativeStringArray do
  it 'keeps NUL-terminated native strings and a trailing null pointer' do
    strings = described_class.new(%w[first second])
    pointer_array = strings.to_ptr

    expect(pointer_array[0].read_pointer.read_string).to eq('first')
    expect(pointer_array[1].read_pointer.read_string).to eq('second')
    expect(pointer_array[2].read_pointer).to be_null
  end

  it 'rejects embedded NUL bytes' do
    expect { described_class.new(["invalid\0string"]) }
      .to raise_error(ArgumentError, /NUL/)
  end
end

# frozen_string_literal: true

RSpec.describe LLDB::FileSpecList do
  let(:first) { LLDB::FileSpec.new('/tmp/first.c') }
  let(:second) { LLDB::FileSpec.new('/tmp/second.c') }

  it 'stores and retrieves FileSpec objects' do
    list = described_class.new

    expect(list).to be_valid
    list.append(first)
    list.append(second)

    expect(list.size).to eq(2)
    expect(list.to_a.map(&:path)).to eq(['/tmp/first.c', '/tmp/second.c'])
  end

  it 'can append only unique file specs and clear the list' do
    list = described_class.new

    expect(list.append_if_unique(first)).to be true
    expect(list.append_if_unique(first)).to be false
    expect(list.size).to eq(1)

    list.clear
    expect(list.size).to eq(0)
  end
end

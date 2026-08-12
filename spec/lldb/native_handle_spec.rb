# frozen_string_literal: true

RSpec.describe LLDB::NativeHandle do
  let(:pointer) { FFI::Pointer.new(0x1234) }

  it 'releases an explicit handle once and returns a null pointer afterwards' do
    releases = []
    handle = described_class.new(pointer, release: ->(released) { releases << released })

    expect(handle.to_ptr).to eq(pointer)
    expect(handle.close).to be true
    expect(handle.close).to be false
    expect(handle.closed?).to be true
    expect(handle.to_ptr).to be_null
    expect(releases).to eq([pointer])
  end

  it 'does not release twice when close races across Ruby threads' do
    releases = []
    handle = described_class.new(pointer, release: ->(released) { releases << released })

    threads = 8.times.map { Thread.new { handle.close } }
    threads.each(&:join)

    expect(releases).to eq([pointer])
  end

  it 'releases an unclosed handle during finalization' do
    releases = []
    state = [[pointer, ->(released) { releases << released }]]
    finalizer = described_class.finalizer(state)

    finalizer.call(0)
    finalizer.call(0)

    expect(releases).to eq([pointer])
  end

  it 'releases an unclosed handle during actual garbage collection' do
    released = 0
    previous_stress = GC.stress

    begin
      GC.stress = true
      handle = described_class.new(pointer, release: ->(_) { released += 1 })
      handle = nil
      GC.start
    ensure
      GC.stress = previous_stress
    end

    expect(released).to eq(1)
  end
end

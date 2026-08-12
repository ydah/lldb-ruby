# frozen_string_literal: true

RSpec.describe LLDB::Context do
  it 'tracks open, closing, and closed state while draining children' do
    context = described_class.new
    releases = []
    first = context.register(LLDB::NativeHandle.new(FFI::Pointer.new(1), release: ->(_) { releases << :first }))
    context.register(LLDB::NativeHandle.new(FFI::Pointer.new(2), release: ->(_) { releases << :second }))

    expect(context).to be_open
    context.close(except: first)

    expect(context).to be_closed
    expect(releases).to eq([:second])
    expect(first).not_to be_closed
    first.close
    expect(releases).to eq(%i[second first])
  end

  it 'does not keep registered handles alive' do
    context = described_class.new
    releases = []
    context.register(LLDB::NativeHandle.new(FFI::Pointer.new(3), release: ->(_) { releases << :released }))

    3.times do
      GC.start
      break unless releases.empty?
    end

    expect(releases).to eq([:released])
  end
end

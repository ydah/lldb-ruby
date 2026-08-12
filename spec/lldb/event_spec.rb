# frozen_string_literal: true

RSpec.describe LLDB::Broadcaster do
  let(:broadcaster) { described_class.new('spec broadcaster') }
  let(:listener) { LLDB::Listener.new('spec listener') }

  after do
    listener.close if listener&.valid?
    broadcaster.close if broadcaster&.valid?
  end

  it 'delivers a broadcast event without a background thread' do
    expect(listener.start_listening_for_events(broadcaster, 0x1)).to eq(1)
    broadcaster.broadcast_event_by_type(0x1)

    event = listener.wait_for_event(timeout_seconds: 0)
    expect(event).to be_a(LLDB::Event)
    expect(event).to be_valid
    expect(event.type).to eq(0x1)
    expect(event.broadcaster_class).to eq('lldb.anonymous')
    expect(event.description).to include('spec broadcaster')
    expect(event.broadcaster).to be_a(LLDB::Broadcaster)
    event.close
  end

  it 'distinguishes timeout, peek, and next event operations' do
    expect(listener.wait_for_event(timeout_seconds: 0)).to be_nil

    listener.start_listening_for_events(broadcaster, 0x2)
    broadcaster.broadcast_event_by_type(0x2)

    peeked = listener.peek_event
    expect(peeked).to be_valid
    expect(listener.peek_event).to be_valid
    expect(listener.next_event.type).to eq(0x2)
    expect(listener.next_event).to be_nil
    peeked.close
  end

  it 'validates timeout values' do
    expect { listener.wait_for_event(timeout_seconds: -1) }.to raise_error(ArgumentError)
    expect { listener.wait_for_event(timeout_seconds: 0x1_0000_0000) }.to raise_error(ArgumentError)
  end

  it 'tracks broadcaster listener masks' do
    expect(broadcaster.add_listener(listener, 0x4)).to eq(0x4)
    expect(broadcaster.event_type_has_listeners?(0x4)).to be true
    expect(broadcaster.remove_listener(listener, 0x4)).to be true
    expect(broadcaster.event_type_has_listeners?(0x4)).to be false
  end
end

RSpec.describe LLDB::Debugger do
  it 'exposes debugger listener and broadcaster handles' do
    debugger = described_class.create

    expect(debugger.listener).to be_a(LLDB::Listener)
    expect(debugger.broadcaster).to be_a(LLDB::Broadcaster)
  end
end

RSpec.describe 'process events' do
  let(:debugger) { LLDB::Debugger.create }
  let(:target) do
    debugger.async = false
    target = debugger.create_target(compile_fixture('simple'))
    target.breakpoint_create_by_name('main')
    target
  end
  let(:process) { target.launch }

  after do
    process.kill if process&.valid?
  end

  it 'exposes process state through a queued event' do
    listener = debugger.listener
    broadcaster = process.broadcaster
    expect(listener.start_listening_for_events(broadcaster, LLDB::Process::BroadcastBit::STATE_CHANGED)).to eq(1)

    process.continue
    event = listener.wait_for_event(timeout_seconds: 0)

    expect(event).to be_a(LLDB::Event)
    expect(event).to be_process_event
    expect([LLDB::State::RUNNING, LLDB::State::EXITED]).to include(event.process_state)
    expect(event.process).to be_a(LLDB::Process)
    expect(event.process).to be_valid
    event.close
    broadcaster.close
    listener.close
  end
end

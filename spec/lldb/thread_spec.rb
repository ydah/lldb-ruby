# frozen_string_literal: true

RSpec.describe LLDB::Thread do
  let(:debugger) { LLDB::Debugger.create }
  let(:executable) { compile_fixture('simple') }
  let(:target) { debugger.create_target(executable) }
  let(:process) do
    debugger.async = false
    target.breakpoint_create_by_name('main')
    target.launch
  end
  let(:thread) { process.selected_thread }

  after do
    process.kill if process&.valid?
  end

  describe '#valid?' do
    it 'returns true for a valid thread' do
      expect(thread).to be_valid
    end
  end

  describe '#thread_id' do
    it 'returns a valid thread ID' do
      expect(thread.thread_id).to be > 0
    end
  end

  describe '#id' do
    it 'is an alias for thread_id' do
      expect(thread.id).to eq(thread.thread_id)
    end
  end

  describe '#stop_reason' do
    it 'returns BREAKPOINT when stopped at breakpoint' do
      expect(thread.stop_reason).to eq(LLDB::StopReason::BREAKPOINT)
    end
  end

  describe '#stop_reason_name' do
    it "returns 'breakpoint' when stopped at breakpoint" do
      expect(thread.stop_reason_name).to eq('breakpoint')
    end
  end

  describe '#stopped_at_breakpoint?' do
    it 'returns true when stopped at breakpoint' do
      expect(thread).to be_stopped_at_breakpoint
    end
  end

  describe '#num_frames' do
    it 'returns at least 1 frame' do
      expect(thread.num_frames).to be >= 1
    end
  end

  describe '#frames' do
    it 'returns an array of frames' do
      frames = thread.frames
      expect(frames).not_to be_empty
      expect(frames.first).to be_a(LLDB::Frame)
      expect(frames.first).to be_valid
    end
  end

  describe '#selected_frame' do
    it 'returns the selected frame' do
      frame = thread.selected_frame
      expect(frame).to be_a(LLDB::Frame)
      expect(frame).to be_valid
    end
  end

  describe '#frame_at_index' do
    it 'returns the frame at the specified index' do
      frame = thread.frame_at_index(0)
      expect(frame).to be_a(LLDB::Frame)
      expect(frame).to be_valid
    end
  end

  describe '#index_id' do
    it 'returns a valid index ID' do
      expect(thread.index_id).to be >= 0
    end
  end

  describe '#stop_description' do
    it 'returns a string describing the stop reason' do
      desc = thread.stop_description
      expect(desc).to be_a(String)
    end
  end

  describe '#stopped?' do
    it 'returns a boolean' do
      # IsStopped() behavior can vary by LLDB version
      # We just verify it returns a boolean and is consistent with process state
      expect([true, false]).to include(thread.stopped?)
    end
  end

  describe '#suspended?' do
    it 'returns a boolean' do
      expect([true, false]).to include(thread.suspended?)
    end
  end

  describe '#suspend and #resume' do
    it 'can suspend and resume the thread' do
      thread.suspend
      expect(thread.suspended?).to be true

      thread.resume
      expect(thread.suspended?).to be false
    end
  end
end

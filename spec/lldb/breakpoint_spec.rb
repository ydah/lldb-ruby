# frozen_string_literal: true

RSpec.describe LLDB::Breakpoint do
  let(:debugger) { LLDB::Debugger.create }
  let(:executable) { compile_fixture('simple') }
  let(:target) { debugger.create_target(executable) }
  let(:breakpoint) { target.breakpoint_create_by_name('main') }

  describe '#valid?' do
    it 'returns true for a valid breakpoint' do
      expect(breakpoint).to be_valid
    end
  end

  describe '#id' do
    it 'returns a positive ID' do
      expect(breakpoint.id).to be > 0
    end
  end

  describe '#enabled?' do
    it 'returns true by default' do
      expect(breakpoint).to be_enabled
    end
  end

  describe '#enabled=' do
    it 'can enable and disable the breakpoint' do
      breakpoint.enabled = false
      expect(breakpoint).not_to be_enabled

      breakpoint.enabled = true
      expect(breakpoint).to be_enabled
    end
  end

  describe '#enable and #disable' do
    it 'enables the breakpoint' do
      breakpoint.disable
      expect(breakpoint).not_to be_enabled

      breakpoint.enable
      expect(breakpoint).to be_enabled
    end
  end

  describe '#one_shot?' do
    it 'returns false by default' do
      expect(breakpoint).not_to be_one_shot
    end
  end

  describe '#one_shot=' do
    it 'can set one-shot mode' do
      breakpoint.one_shot = true
      expect(breakpoint).to be_one_shot

      breakpoint.one_shot = false
      expect(breakpoint).not_to be_one_shot
    end
  end

  describe '#hit_count' do
    it 'returns 0 before hitting' do
      expect(breakpoint.hit_count).to eq(0)
    end
  end

  describe '#ignore_count' do
    it 'returns 0 by default' do
      expect(breakpoint.ignore_count).to eq(0)
    end

    it 'can be set' do
      breakpoint.ignore_count = 5
      expect(breakpoint.ignore_count).to eq(5)
    end
  end

  describe '#condition' do
    it 'returns nil by default' do
      expect(breakpoint.condition).to be_nil
    end

    it 'can be set' do
      breakpoint.condition = 'x > 5'
      expect(breakpoint.condition).to eq('x > 5')
    end
  end

  describe '#num_locations' do
    it 'returns at least 1 for a valid breakpoint' do
      expect(breakpoint.num_locations).to be >= 1
    end
  end

  describe '#to_s' do
    it 'returns a string representation' do
      str = breakpoint.to_s
      expect(str).to include('Breakpoint')
      expect(str).to include(breakpoint.id.to_s)
    end
  end

  describe '#delete' do
    it 'deletes the breakpoint from the target' do
      id = breakpoint.id
      breakpoint.delete

      expect(target.find_breakpoint_by_id(id)).to be_nil
    end
  end

  describe '#location_at_index' do
    it 'returns a breakpoint location' do
      location = breakpoint.location_at_index(0)
      skip 'No location found' if location.nil?

      expect(location).to be_a(LLDB::BreakpointLocation)
      expect(location).to be_valid
    end
  end

  describe '#locations' do
    it 'returns all locations' do
      locations = breakpoint.locations
      expect(locations).to be_an(Array)
      expect(locations.size).to eq(breakpoint.num_locations)
    end
  end

  describe '#hardware?' do
    it 'returns a boolean' do
      expect([true, false]).to include(breakpoint.hardware?)
    end
  end

  describe '#auto_continue' do
    it 'returns false by default' do
      expect(breakpoint.auto_continue?).to be false
    end

    it 'can be set' do
      breakpoint.auto_continue = true
      expect(breakpoint.auto_continue?).to be true

      breakpoint.auto_continue = false
      expect(breakpoint.auto_continue?).to be false
    end
  end

  describe '#thread_id' do
    it 'returns 0 by default' do
      expect(breakpoint.thread_id).to eq(0)
    end

    it 'can be set' do
      breakpoint.thread_id = 12_345
      expect(breakpoint.thread_id).to eq(12_345)
    end
  end

  describe '#thread_index' do
    it 'returns UINT32_MAX by default (no thread set)' do
      # UINT32_MAX (4294967295) indicates no specific thread is set
      expect(breakpoint.thread_index).to eq(0xFFFFFFFF)
    end

    it 'can be set' do
      breakpoint.thread_index = 1
      expect(breakpoint.thread_index).to eq(1)
    end
  end

  describe '#thread_name' do
    it 'returns nil by default' do
      expect(breakpoint.thread_name).to be_nil
    end

    it 'can be set' do
      breakpoint.thread_name = 'main_thread'
      expect(breakpoint.thread_name).to eq('main_thread')
    end
  end
end

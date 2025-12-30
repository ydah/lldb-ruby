# frozen_string_literal: true

RSpec.describe LLDB::BreakpointLocation do
  let(:debugger) { LLDB::Debugger.create }
  let(:executable) { compile_fixture('simple') }
  let(:target) { debugger.create_target(executable) }
  let(:breakpoint) { target.breakpoint_create_by_name('main') }
  let(:location) { breakpoint.location_at_index(0) }

  describe '#valid?' do
    it 'returns true for a valid location' do
      skip 'No location found' if location.nil?
      expect(location).to be_valid
    end
  end

  describe '#id' do
    it 'returns a positive ID' do
      skip 'No location found' if location.nil?
      expect(location.id).to be > 0
    end
  end

  describe '#load_address' do
    it 'returns the load address' do
      skip 'No location found' if location.nil?
      expect(location.load_address).to be >= 0
    end
  end

  describe '#enabled?' do
    it 'returns true by default' do
      skip 'No location found' if location.nil?
      expect(location).to be_enabled
    end
  end

  describe '#enabled=' do
    it 'can enable and disable the location' do
      skip 'No location found' if location.nil?

      location.enabled = false
      expect(location).not_to be_enabled

      location.enabled = true
      expect(location).to be_enabled
    end
  end

  describe '#hit_count' do
    it 'returns 0 before hitting' do
      skip 'No location found' if location.nil?
      expect(location.hit_count).to eq(0)
    end
  end

  describe '#ignore_count' do
    it 'returns 0 by default' do
      skip 'No location found' if location.nil?
      expect(location.ignore_count).to eq(0)
    end

    it 'can be set' do
      skip 'No location found' if location.nil?
      location.ignore_count = 3
      expect(location.ignore_count).to eq(3)
    end
  end

  describe '#condition' do
    it 'returns nil by default' do
      skip 'No location found' if location.nil?
      expect(location.condition).to be_nil
    end

    it 'can be set' do
      skip 'No location found' if location.nil?
      location.condition = 'x == 42'
      expect(location.condition).to eq('x == 42')
    end
  end

  describe '#to_s' do
    it 'returns a string representation' do
      skip 'No location found' if location.nil?
      str = location.to_s
      expect(str).to include('BreakpointLocation')
      expect(str).to include(location.id.to_s)
    end
  end
end

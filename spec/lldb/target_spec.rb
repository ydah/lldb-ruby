# frozen_string_literal: true

RSpec.describe LLDB::Target do
  let(:debugger) { LLDB::Debugger.create }
  let(:executable) { compile_fixture('simple') }
  let(:target) { debugger.create_target(executable) }

  describe '#valid?' do
    it 'returns true for a valid target' do
      expect(target).to be_valid
    end
  end

  describe '#executable_path' do
    it 'returns the path to the executable' do
      expect(target.executable_path).to eq(executable)
    end
  end

  describe '#breakpoint_create_by_name' do
    it 'creates a breakpoint by function name' do
      bp = target.breakpoint_create_by_name('main')
      expect(bp).to be_a(LLDB::Breakpoint)
      expect(bp).to be_valid
      expect(bp.id).to be > 0
    end

    it 'creates a breakpoint for a specific function' do
      bp = target.breakpoint_create_by_name('lldb_test_add')
      expect(bp).to be_valid
      expect(bp.num_locations).to be >= 1
    end
  end

  describe '#breakpoint_create_by_location' do
    it 'creates a breakpoint by file and line' do
      bp = target.breakpoint_create_by_location('simple.c', 4)
      expect(bp).to be_a(LLDB::Breakpoint)
      expect(bp).to be_valid
    end
  end

  describe '#num_breakpoints' do
    it 'returns 0 for a new target' do
      expect(target.num_breakpoints).to eq(0)
    end

    it 'returns the number of breakpoints' do
      target.breakpoint_create_by_name('main')
      target.breakpoint_create_by_name('lldb_test_add')
      expect(target.num_breakpoints).to eq(2)
    end
  end

  describe '#breakpoints' do
    it 'returns all breakpoints' do
      target.breakpoint_create_by_name('main')
      target.breakpoint_create_by_name('lldb_test_add')

      breakpoints = target.breakpoints
      expect(breakpoints.length).to eq(2)
      expect(breakpoints).to all(be_valid)
    end
  end

  describe '#find_breakpoint_by_id' do
    it 'finds a breakpoint by ID' do
      bp = target.breakpoint_create_by_name('main')
      found = target.find_breakpoint_by_id(bp.id)

      expect(found).to be_valid
      expect(found.id).to eq(bp.id)
    end

    it 'returns nil for non-existent ID' do
      found = target.find_breakpoint_by_id(9999)
      expect(found).to be_nil
    end
  end

  describe '#delete_breakpoint' do
    it 'deletes a breakpoint by ID' do
      bp = target.breakpoint_create_by_name('main')
      expect(target.num_breakpoints).to eq(1)

      result = target.delete_breakpoint(bp.id)
      expect(result).to be true
      expect(target.num_breakpoints).to eq(0)
    end
  end

  describe '#num_modules' do
    it 'returns at least 1 for the main executable' do
      expect(target.num_modules).to be >= 1
    end
  end

  describe '#modules' do
    it 'returns loaded modules' do
      modules = target.modules
      expect(modules).not_to be_empty
      expect(modules.first).to be_valid
    end
  end

  describe '#breakpoint_create_by_regex' do
    it 'creates a breakpoint by regex' do
      bp = target.breakpoint_create_by_regex('main')
      expect(bp).to be_a(LLDB::Breakpoint)
      expect(bp).to be_valid
    end
  end

  describe '#delete_all_breakpoints' do
    it 'deletes all breakpoints' do
      target.breakpoint_create_by_name('main')
      target.breakpoint_create_by_name('lldb_test_add')
      expect(target.num_breakpoints).to eq(2)

      result = target.delete_all_breakpoints
      expect(result).to be true
      expect(target.num_breakpoints).to eq(0)
    end
  end

  describe '#enable_all_breakpoints' do
    it 'enables all breakpoints' do
      bp1 = target.breakpoint_create_by_name('main')
      bp2 = target.breakpoint_create_by_name('lldb_test_add')
      bp1.disable
      bp2.disable

      result = target.enable_all_breakpoints
      expect(result).to be true
    end
  end

  describe '#disable_all_breakpoints' do
    it 'disables all breakpoints' do
      target.breakpoint_create_by_name('main')
      target.breakpoint_create_by_name('lldb_test_add')

      result = target.disable_all_breakpoints
      expect(result).to be true
    end
  end

  describe '#triple' do
    it 'returns the target triple' do
      triple = target.triple
      expect(triple).to be_a(String)
      expect(triple).not_to be_empty
    end
  end

  describe '#address_byte_size' do
    it 'returns the address byte size' do
      size = target.address_byte_size
      expect(size).to be_a(Integer)
      expect([4, 8]).to include(size)
    end
  end

  describe 'watchpoint methods' do
    it 'has num_watchpoints returning 0 for a new target' do
      expect(target.num_watchpoints).to eq(0)
    end

    it 'has empty watchpoints array for a new target' do
      expect(target.watchpoints).to eq([])
    end
  end
end

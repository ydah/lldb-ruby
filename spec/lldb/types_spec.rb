# frozen_string_literal: true

RSpec.describe LLDB::State do
  describe '.name' do
    it "returns 'stopped' for STOPPED state" do
      expect(LLDB::State.name(LLDB::State::STOPPED)).to eq('stopped')
    end

    it "returns 'running' for RUNNING state" do
      expect(LLDB::State.name(LLDB::State::RUNNING)).to eq('running')
    end

    it "returns 'exited' for EXITED state" do
      expect(LLDB::State.name(LLDB::State::EXITED)).to eq('exited')
    end

    it "returns 'unknown' for unknown state" do
      expect(LLDB::State.name(999)).to eq('unknown')
    end
  end
end

RSpec.describe LLDB::StopReason do
  describe '.name' do
    it "returns 'breakpoint' for BREAKPOINT reason" do
      expect(LLDB::StopReason.name(LLDB::StopReason::BREAKPOINT)).to eq('breakpoint')
    end

    it "returns 'signal' for SIGNAL reason" do
      expect(LLDB::StopReason.name(LLDB::StopReason::SIGNAL)).to eq('signal')
    end

    it "returns 'unknown' for unknown reason" do
      expect(LLDB::StopReason.name(999)).to eq('unknown')
    end
  end
end

RSpec.describe LLDB::ValueType do
  describe 'constants' do
    it 'has INVALID constant' do
      expect(LLDB::ValueType::INVALID).to eq(0)
    end

    it 'has VARIABLE_LOCAL constant' do
      expect(LLDB::ValueType::VARIABLE_LOCAL).to eq(4)
    end

    it 'has REGISTER constant' do
      expect(LLDB::ValueType::REGISTER).to eq(5)
    end
  end

  describe '.name' do
    it "returns 'local' for VARIABLE_LOCAL" do
      expect(LLDB::ValueType.name(LLDB::ValueType::VARIABLE_LOCAL)).to eq('local')
    end

    it "returns 'global' for VARIABLE_GLOBAL" do
      expect(LLDB::ValueType.name(LLDB::ValueType::VARIABLE_GLOBAL)).to eq('global')
    end

    it "returns 'register' for REGISTER" do
      expect(LLDB::ValueType.name(LLDB::ValueType::REGISTER)).to eq('register')
    end

    it "returns 'unknown' for unknown value type" do
      expect(LLDB::ValueType.name(999)).to eq('unknown')
    end
  end
end

RSpec.describe LLDB::BasicType do
  describe 'constants' do
    it 'has INVALID constant' do
      expect(LLDB::BasicType::INVALID).to eq(0)
    end

    it 'has VOID constant' do
      expect(LLDB::BasicType::VOID).to eq(1)
    end

    it 'has INT constant' do
      expect(LLDB::BasicType::INT).to eq(12)
    end

    it 'has DOUBLE constant' do
      expect(LLDB::BasicType::DOUBLE).to eq(23)
    end
  end

  describe '.name' do
    it "returns 'int' for INT" do
      expect(LLDB::BasicType.name(LLDB::BasicType::INT)).to eq('int')
    end

    it "returns 'void' for VOID" do
      expect(LLDB::BasicType.name(LLDB::BasicType::VOID)).to eq('void')
    end

    it "returns 'double' for DOUBLE" do
      expect(LLDB::BasicType.name(LLDB::BasicType::DOUBLE)).to eq('double')
    end

    it "returns 'unknown' for unknown basic type" do
      expect(LLDB::BasicType.name(999)).to eq('unknown')
    end
  end
end

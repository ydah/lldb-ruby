# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  module State
    INVALID = 0 # : Integer
    UNLOADED = 1 # : Integer
    CONNECTED = 2 # : Integer
    ATTACHING = 3 # : Integer
    LAUNCHING = 4 # : Integer
    STOPPED = 5 # : Integer
    RUNNING = 6 # : Integer
    STEPPING = 7 # : Integer
    CRASHED = 8 # : Integer
    DETACHED = 9 # : Integer
    EXITED = 10 # : Integer
    SUSPENDED = 11 # : Integer

    NAMES = { # : Hash[Integer, String]
      INVALID => 'invalid',
      UNLOADED => 'unloaded',
      CONNECTED => 'connected',
      ATTACHING => 'attaching',
      LAUNCHING => 'launching',
      STOPPED => 'stopped',
      RUNNING => 'running',
      STEPPING => 'stepping',
      CRASHED => 'crashed',
      DETACHED => 'detached',
      EXITED => 'exited',
      SUSPENDED => 'suspended'
    }.freeze

    # @rbs state: Integer
    # @rbs return: String
    def self.name(state)
      NAMES[state] || 'unknown'
    end
  end

  module StopReason
    INVALID = 0 # : Integer
    NONE = 1 # : Integer
    TRACE = 2 # : Integer
    BREAKPOINT = 3 # : Integer
    WATCHPOINT = 4 # : Integer
    SIGNAL = 5 # : Integer
    EXCEPTION = 6 # : Integer
    EXEC = 7 # : Integer
    PLAN_COMPLETE = 8 # : Integer
    THREAD_EXITING = 9 # : Integer
    INSTRUMENTATION = 10 # : Integer

    NAMES = { # : Hash[Integer, String]
      INVALID => 'invalid',
      NONE => 'none',
      TRACE => 'trace',
      BREAKPOINT => 'breakpoint',
      WATCHPOINT => 'watchpoint',
      SIGNAL => 'signal',
      EXCEPTION => 'exception',
      EXEC => 'exec',
      PLAN_COMPLETE => 'plan complete',
      THREAD_EXITING => 'thread exiting',
      INSTRUMENTATION => 'instrumentation'
    }.freeze

    # @rbs reason: Integer
    # @rbs return: String
    def self.name(reason)
      NAMES[reason] || 'unknown'
    end
  end

  module SymbolContextItem
    TARGET = 1 << 0 # : Integer
    MODULE = 1 << 1 # : Integer
    COMPILE_UNIT = 1 << 2 # : Integer
    FUNCTION = 1 << 3 # : Integer
    BLOCK = 1 << 4 # : Integer
    LINE_ENTRY = 1 << 5 # : Integer
    SYMBOL = 1 << 6 # : Integer
    EVERYTHING = 0xFFFF # : Integer
  end

  module ValueType
    INVALID = 0 # : Integer
    VARIABLE_GLOBAL = 1 # : Integer
    VARIABLE_STATIC = 2 # : Integer
    VARIABLE_ARGUMENT = 3 # : Integer
    VARIABLE_LOCAL = 4 # : Integer
    REGISTER = 5 # : Integer
    REGISTER_SET = 6 # : Integer
    CONSTANT_RESULT = 7 # : Integer

    NAMES = { # : Hash[Integer, String]
      INVALID => 'invalid',
      VARIABLE_GLOBAL => 'global',
      VARIABLE_STATIC => 'static',
      VARIABLE_ARGUMENT => 'argument',
      VARIABLE_LOCAL => 'local',
      REGISTER => 'register',
      REGISTER_SET => 'register set',
      CONSTANT_RESULT => 'constant result'
    }.freeze

    # @rbs value_type: Integer
    # @rbs return: String
    def self.name(value_type)
      NAMES[value_type] || 'unknown'
    end
  end

  module BasicType
    INVALID = 0 # : Integer
    VOID = 1 # : Integer
    CHAR = 2 # : Integer
    SIGNED_CHAR = 3 # : Integer
    UNSIGNED_CHAR = 4 # : Integer
    WCHAR = 5 # : Integer
    SIGNED_WCHAR = 6 # : Integer
    UNSIGNED_WCHAR = 7 # : Integer
    CHAR16 = 8 # : Integer
    CHAR32 = 9 # : Integer
    SHORT = 10 # : Integer
    UNSIGNED_SHORT = 11 # : Integer
    INT = 12 # : Integer
    UNSIGNED_INT = 13 # : Integer
    LONG = 14 # : Integer
    UNSIGNED_LONG = 15 # : Integer
    LONG_LONG = 16 # : Integer
    UNSIGNED_LONG_LONG = 17 # : Integer
    INT128 = 18 # : Integer
    UNSIGNED_INT128 = 19 # : Integer
    BOOL = 20 # : Integer
    HALF = 21 # : Integer
    FLOAT = 22 # : Integer
    DOUBLE = 23 # : Integer
    LONG_DOUBLE = 24 # : Integer
    FLOAT_COMPLEX = 25 # : Integer
    DOUBLE_COMPLEX = 26 # : Integer
    LONG_DOUBLE_COMPLEX = 27 # : Integer
    OBJ_C_ID = 28 # : Integer
    OBJ_C_CLASS = 29 # : Integer
    OBJ_C_SEL = 30 # : Integer
    NULL_PTR = 31 # : Integer

    NAMES = { # : Hash[Integer, String]
      INVALID => 'invalid',
      VOID => 'void',
      CHAR => 'char',
      SIGNED_CHAR => 'signed char',
      UNSIGNED_CHAR => 'unsigned char',
      WCHAR => 'wchar_t',
      SIGNED_WCHAR => 'signed wchar_t',
      UNSIGNED_WCHAR => 'unsigned wchar_t',
      CHAR16 => 'char16_t',
      CHAR32 => 'char32_t',
      SHORT => 'short',
      UNSIGNED_SHORT => 'unsigned short',
      INT => 'int',
      UNSIGNED_INT => 'unsigned int',
      LONG => 'long',
      UNSIGNED_LONG => 'unsigned long',
      LONG_LONG => 'long long',
      UNSIGNED_LONG_LONG => 'unsigned long long',
      INT128 => '__int128',
      UNSIGNED_INT128 => 'unsigned __int128',
      BOOL => 'bool',
      HALF => 'half',
      FLOAT => 'float',
      DOUBLE => 'double',
      LONG_DOUBLE => 'long double',
      FLOAT_COMPLEX => 'float complex',
      DOUBLE_COMPLEX => 'double complex',
      LONG_DOUBLE_COMPLEX => 'long double complex',
      OBJ_C_ID => 'id',
      OBJ_C_CLASS => 'Class',
      OBJ_C_SEL => 'SEL',
      NULL_PTR => 'nullptr_t'
    }.freeze

    # @rbs basic_type: Integer
    # @rbs return: String
    def self.name(basic_type)
      NAMES[basic_type] || 'unknown'
    end
  end
end

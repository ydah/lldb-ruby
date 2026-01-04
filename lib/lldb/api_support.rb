# frozen_string_literal: true

# rbs_inline: enabled

module LLDB
  # Module for checking API availability across different LLDB versions
  # and lldb-ruby binding implementations.
  module APISupport
    # Feature to FFI method mapping
    FEATURES = {
      breakpoint_by_address: :lldb_target_breakpoint_create_by_address,
      breakpoint_by_regex: :lldb_target_breakpoint_create_by_regex,
      memory_read: :lldb_process_read_memory,
      memory_write: :lldb_process_write_memory,
      thread_by_id: :lldb_process_get_thread_by_id,
      memory_region_info: :lldb_process_get_memory_region_info,
      step_into: :lldb_thread_step_into,
      step_over: :lldb_thread_step_over,
      step_out: :lldb_thread_step_out,
      step_instruction: :lldb_thread_step_instruction,
      registers: :lldb_frame_get_registers,
      find_variable: :lldb_frame_find_variable,
      evaluate_expression: :lldb_frame_evaluate_expression
    }.freeze

    class << self
      # Check if a specific FFI method is available
      #
      # @rbs method_name: Symbol
      # @rbs return: bool
      def method_available?(method_name)
        FFIBindings.respond_to?(method_name)
      end

      # Raise UnsupportedAPIError if the required method is not available
      #
      # @rbs method_name: Symbol
      # @rbs feature_name: String?
      # @rbs return: void
      def require_method!(method_name, feature_name = nil)
        return if method_available?(method_name)

        feature = feature_name || method_name.to_s
        raise UnsupportedAPIError,
              "API '#{feature}' is not supported in this LLDB version or binding"
      end

      # Check if a feature is supported
      #
      # @rbs feature: Symbol
      # @rbs return: bool
      def feature_supported?(feature)
        method_name = FEATURES[feature]
        return false unless method_name

        method_available?(method_name)
      end

      # Get list of all supported features
      #
      # @rbs return: Array[Symbol]
      def supported_features
        FEATURES.keys.select { |f| feature_supported?(f) }
      end

      # Get list of all unsupported features
      #
      # @rbs return: Array[Symbol]
      def unsupported_features
        FEATURES.keys.reject { |f| feature_supported?(f) }
      end
    end
  end
end

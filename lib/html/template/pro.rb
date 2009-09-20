require 'html_template_internal'

module HTML
  module Template
    class Pro

      include HTML::Template::Internal

      def initialize(args={})
        @options = default_options.merge(args)
        @params = @options[:param_map]
        if args.include? :filename
          @filename = args[:filename]
        end
      end

      def param(params)
        @params.update(params)
      end

      def clear_params
        @params.clear
      end

      def output(options={})
        if (options.include? :print_to)
          exec_tmpl(options[:print_to])
        else
          output_string = String.new
          exec_tmpl(output_string)
          return output_string
        end
      end

      private

      def default_options
        return {
          :param_map => {},
          :filter => [],
          :debug => 0,
          :max_includes => 10,
          :global_vars => 0,
          :no_includes => 0,
          :search_path_on_include => 0,
          :loop_context_vars => 0,
          :path => [],
          :associate => [],
          :case_sensitive => 0,
          :strict => 1,
          :die_on_bad_params => 0,
        }
      end
    end
  end
end

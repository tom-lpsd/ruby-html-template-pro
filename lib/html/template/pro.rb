require 'html_template_internal'

module HTML
  module Template
    class Pro

      include HTML::Template::Internal

      INPUTS = [:filename, :filehandle, :arrayref, :scalarref, :source]

      def initialize(args={})
        @options = default_options.merge(args)
        if args.keys.count(&INPUTS.method(:include?)) != 1
          raise "HTML::Template::Pro.new called with multiple (or no) template sources specified!"
        end
        @params = @options[:param_map]
        [:path, :associate, :filter].each do |opt|
          unless @options[opt].instance_of? Array
            @options[opt] = [ @options[opt] ]
          end
        end
        initialize_tmpl_source args
        if @scalarref and @options[:filter]
          @scalarref = call_filters @scalarref
        end
        @filtered_template = {}
      end

      def param(args=nil)
        if args.nil?
          return @params.keys
        elsif !(args.instance_of? Hash)
          key = @options[:case_sensitive] ? args : args.downcase
          return @params[key] || @params[args]
        end
        merge_params(args)
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
          :debug => false,
          :max_includes => 10,
          :global_vars => false,
          :no_includes => false,
          :search_path_on_include => false,
          :loop_context_vars => false,
          :path_like_variable_scope => false,
          :path => [],
          :associate => [],
          :case_sensitive => false,
          :strict => true,
          :die_on_bad_params => false,
        }
      end

      def initialize_tmpl_source(args)
        if args.include? :filename
          @filename = args[:filename]
          @scalarref = nil
          return
        end

        @filename = nil
        if args.include? :source
          source = args[:source]
          @scalarref = case source
                       when IO     then source.read
                       when Array  then source.join('')
                       when String then source
                       else
                         if source.respond_to? :to_str 
                           source.to_str
                         else
                           raise "unknown source type"
                         end
                       end
        elsif args.include? :scalarref
          @scalarref = args[:scalarref]
        elsif args.include? :arrayref
          @scalarref = args[:arrayref].join('')
        elsif args.include? :filehandle
          @scalarref = args[:filehandle].read
        end
      end

      def merge_params(params)
        unless @options[:case_sensitive]
          params = lowercase_keys params
        end
        @params.update(params)
      end

      def lowercase_keys(orighash)
        Hash[
             orighash.map do |key, val|
               case val
               when Array then [key.downcase, val.map(&(method :lowercase_keys))]
               when Proc  then [key.downcase, val.call]
               else [key.downcase, val]
               end
             end
            ]
      end

      def load_template(filepath)
        File.open(filepath, 'r') do |file|
          # filtered template is used in internal. we store it to `self'
          # to prevent gc.
          @filtered_template[filepath] = call_filters file.read
        end
      end

      def call_filters(template)
        @options[:filter].each do |filter|
          format, sub = case filter
                        when Hash then filter.values_at(:format, :sub)
                        when Proc then ['scalar', filter]
                        else raise "bad value set for filter parameter - must be a Proc or a Hash object."
                        end

          unless format and sub
            raise "bad value set for filter parameter - hash must contain \"format\" key and \"sub\" key."
          end
          unless format == 'array' or format == 'scalar'
            raise "bad value set for filter parameter - \"format\" must be either 'array' or 'scalar'"
          end
          unless sub.instance_of? Proc
            raise "bad value set for filter parameter - \"sub\" must be a code ref"
          end

          if format == 'scalar'
            template = sub.call(template)
          else
            template = sub.call(template.split("\n").map {|str| str + "\n"}).join('')
          end
        end
        return template
      end
    end

    # alias
    Expr = Pro
  end
end

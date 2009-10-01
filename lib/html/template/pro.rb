# -*- coding: utf-8 -*-
require 'html/template/internal'

module HTML
  module Template

    # internally used
    def Internal.register_functions_impl(registory, func_spec, &block)
      if block && func_spec.kind_of?(Symbol)
        registory[func_spec] = block
      elsif func_spec.kind_of? Hash
        unless func_spec.values.all?{|e| e.kind_of? Proc}
          raise ArgumentError, "functions must be kind_of Proc"
        end
        registory.update(func_spec)
      else
        raise ArgumentError, "first argument must be symbol or hash contains functions"
      end
    end

    class Pro

      VERSION = "0.0.2"

      ASK_NAME_DEFAULT   = 0
      ASK_NAME_AS_IS     = 1
      ASK_NAME_LOWERCASE = 2
      ASK_NAME_UPPERCASE = 4
      ASK_NAME_MASK = ASK_NAME_AS_IS | ASK_NAME_LOWERCASE | ASK_NAME_UPPERCASE

      @@func = {
        # note that length,defined,sin,cos,log,tan,... are built-in
        :sprintf => lambda { |*args| sprintf(*args) },
        :substr  => lambda { |str, *args| args.size == 2 ? str[*args] : str[args[0]..-1] },
        :lc      => lambda { |str| str.downcase },
        :lcfirst => lambda { |str| str[0,1].downcase + str[1..-1] },
        :uc      => lambda { |str| str.upcase },
        :ucfirst => lambda { |str| str.capitalize },
#        :length  => lambda { |str| str.length },
#        :defined => lambda { |obj| obj.nil? },
#        :abs     => lambda { |num| num.abs },
#        :hex     => lambda { |obj| obj.to_s.hex },
#        :oct     => lambda { |obj| obj.to_s.oct },
        :rand    => lambda { |num|  rand num },
        :srand   => lambda { |seed| srand seed },
      }

      # Create a Template object. Template source is specified by several way.
      # Exactly one kind of source must be specified.
      # 
      # * specify filename
      #
      #     template = HTML::Template::Pro.new(:filename => 'foo.tmpl')
      #
      # * specify source as string
      #
      #     template = HTLM::Template::Pro.new(:source => '<html><body><TMPL_VAR NAME="hello">!!</body></html>')
      #
      # * specify source by array of string
      #
      #     template = HTLM::Template::Pro.new(:source => ['<html>', '<body><TMPL_VAR NAME="hello">!!</body>', '</html>'])
      #
      # * specify IO object
      #
      #     template = HTLM::Template::Pro.new(:source => $stdin)
      # 
      # Other options:
      #
      # [path]
      #
      #   Type: Array of path string.
      #
      #   Example:
      #     # find out tmpl/foo.tmpl
      #     template = HTLM::Template::Pro.new(:filename => 'foo.tmpl', :path => ['tmpl'])
      #
      # [search_path_on_include]
      #
      #   Type: boolean
      #
      def initialize(args={})
        @options = default_options.merge(args)
        if args.keys.count(&INPUTS.method(:include?)) != 1
          raise ArgumentError, "HTML::Template::Pro.new called with multiple (or no) template sources specified!"
        end
        @params = @options[:param_map]
        [:path, :associate, :filter].each do |opt|
          unless @options[opt].kind_of? Array
            @options[opt] = [ @options[opt] ]
          end
        end
        @options[:expr_func] = @@func.merge(@options[:functions] || {})
        initialize_tmpl_source args
        if @scalarref and @options[:filter]
          @scalarref = call_filters @scalarref
        end
        @filtered_template = {}
      end

      def param(args=nil, &block)
        return @params.keys if args.nil?
        if !(args.kind_of? Hash)
          key = @options[:case_sensitive] ? args : args.downcase
          if block
            return @params[key] = block
          else
            return @params[key] || @params[args]
          end
        end
        merge_params(args)
      end

      # Clear the internal param hash. All the parameters set before
      # this call is reset.
      def clear_params
        @params.clear
      end

      # returns the final result of the template. If you want to print
      # out the result, you can do:
      #   puts template.output
      #
      # When `output' is called each occurrence of <TMPL_VAR
      # NAME=name> is replaced with the value assigned to "name" via
      # "param()". If a named parameter is unset it is simply replaced
      # with ’’.  <TMPL_LOOPS> are evaluated once per parameter set,
      # accumulating output on each pass.
      #
      # You may optionally supply a IO object to print:
      #   File.open('result.html', 'w') do |file|
      #      template.output(:print_to => file)
      #   end
      def output(options={})

        @options[:associate].reverse.each do |assoc|
          assoc.param.each do |key|
            param(key => assoc.param(key)) unless @params[key]
          end
        end

        if (options.include? :print_to)
          HTML::Template::Internal.exec_tmpl(self, options[:print_to])
        else
          output_string = String.new
          HTML::Template::Internal.exec_tmpl(self, output_string)
          return output_string
        end
      end

      # <b>for perl compatibility</b>
      #
      # shortcut to HTML::Template::Pro.new(:filehandle => file)
      def self.new_filehandle(file)
        self.new(:filehandle => file)
      end

      # <b>for perl compatibility</b>
      #
      # shortcut to HTML::Template::Pro.new(:filename => filename)
      def self.new_file(filename)
        self.new(:filename => file)
      end

      # <b>for perl compatibility</b>
      #
      # shortcut to HTML::Template::Pro.new(:arrayref => lines)
      def self.new_array_ref(lines)
        self.new(:arrayref => lines)
      end

      # <b>for perl compatibility</b>
      #
      # shortcut to HTML::Template::Pro.new(:scalarref => source_string)
      def self.new_scalar_ref(source)
        self.new(:scalarref => source)
      end

      def self.register_function(func_spec, &block)
        HTML::Template::Internal.register_functions_impl(@@func, func_spec, &block)
      end

      def register_function(func_spec, &block)
        HTML::Template::Internal.register_functions_impl(@options[:expr_func], func_spec, &block)
      end

      private

      INPUTS = [:filename, :filehandle, :arrayref, :scalarref, :source]

      def default_options
        return {
          :param_map => {},
          :filter => [],
          :debug => 0,
          :max_includes => 10,
          :global_vars => false,
          :no_includes => false,
          :search_path_on_include => false,
          :loop_context_vars => false,
          :path_like_variable_scope => false,
          :path => [],
          :associate => [],
          :case_sensitive => false,
          :__strict_compatibility => true,
          :strict => false,
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
               when Proc  then [key.downcase, val]
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
          unless sub.kind_of? Proc
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

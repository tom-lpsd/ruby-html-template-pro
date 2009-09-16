require 'html_template_internal'

module HTML
  module Template
    class Pro

      include HTML::Template::Internal

      def initialize(options={})
        @options = options
        @params = {}
      end
      def param(params)
        @params.update(params)
      end
      def output(options={})
        exec_tmpl(options)
      end
    end
  end
end

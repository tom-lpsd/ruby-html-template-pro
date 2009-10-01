require 'rubygems'
gem 'hoe', '>= 2.1.0'
require 'hoe'
require 'fileutils'

Hoe.plugin :newgem
Hoe.plugin :website
Hoe.add_include_dirs 'ext/html/template'

$hoe = Hoe.spec 'html-template-pro' do
  self.developer 'Tsuruhara Tom', 'tom.lpsd@gmail.com'
  self.rubyforge_name = 'tmplpro'
  self.version = "0.0.2"
  self.summary = 'A Ruby port of HTML::Template::Pro'
  self.extra_rdoc_files = ['README.rdoc']
  spec_extras[:required_ruby_version] = Gem::Requirement.new '> 1.9.0'
end

require 'newgem/tasks'
Dir['tasks/**/*.rake'].each { |t| load t }

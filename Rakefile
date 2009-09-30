require 'rubygems'
gem 'hoe', '>= 2.1.0'
require 'hoe'
require 'fileutils'

Hoe.plugin :newgem
Hoe.plugin :website

$hoe = Hoe.spec 'html-template-pro' do
  self.developer 'Tsuruhara Tom', 'tom.lpsd@gmail.com'
  self.rubyforge_name = 'tmplpro'
  self.version = "0.0.1"
  self.summary = 'A Ruby port of HTML::Template::Pro'
  self.extra_rdoc_files = ['README.rdoc']
end

require 'newgem/tasks'
Dir['tasks/**/*.rake'].each { |t| load t }

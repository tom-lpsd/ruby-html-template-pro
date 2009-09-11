require 'rubygems'
require 'rake/gempackagetask'
require 'rake/extensiontask'

spec = Gem::Specification.new do |s|
  s.name = "ruby-html-template-pro"
  s.platform = Gem::Platform::RUBY
  s.files = FileList["ext/**/*", "lib/**/*"]
  s.extensions = FileList["ext/**/extconf.rb"]
  s.summary = 'ruby port of perl library HTML-Template-Pro'
  s.version = '0.0.1'
end

Rake::GemPackageTask.new(spec) do |pkg|
end

Rake::ExtensionTask.new('html_template_internal', spec)

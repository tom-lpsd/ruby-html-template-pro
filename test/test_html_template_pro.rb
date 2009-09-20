#!/usr/bin/env ruby
# -*- coding: iso-8859-1 -*-

require 'test/unit'
require 'html/template/pro'

class TestHtmlTemplatePro < Test::Unit::TestCase
  def setup
    @varset1 = {
      :VAR1 => 'VAR1', :VAR2 => 'VAR2',
      :VAR3 => 'VAR3', :VAR10 => 'VAR10',
    }
    @varset2 = {
      :STUFF1 => '\<>"; %FA' + "hidden:\r\012end",
      :STUFF2 => 'Áàáà ßãà',
    }
    @refset1 = {
      :HASHREF0 => [],
      :HASHREF2 => [{}, {}],
      :HASHREF1 => [
                    {:LOOPVAR1=>'LOOP1-VAR1',:LOOPVAR2=>'LOOP1-VAR2',:LOOPVAR3=>'LOOP1-VAR3',:LOOPVAR10=>'LOOP1-VAR10'},
                    {:LOOPVAR1=>'LOOP2-VAR1',:LOOPVAR2=>'LOOP2-VAR2',:LOOPVAR3=>'LOOP2-VAR3',:LOOPVAR10=>'LOOP2-VAR10'},
                    {:LOOPVAR1=>'LOOP3-VAR1',:LOOPVAR2=>'LOOP3-VAR2',:LOOPVAR3=>'LOOP3-VAR3',:LOOPVAR10=>'LOOP3-VAR10'},
                    {:LOOPVAR1=>'LOOP4-VAR1',:LOOPVAR2=>'LOOP4-VAR2',:LOOPVAR3=>'LOOP4-VAR3',:LOOPVAR10=>'LOOP4-VAR10'},
                   ],
    }
    @outer=[{:TEST=>'1'},{:TEST=>'2'},{:TEST=>'3'}]
    @inner=[{:TST=>'A'},{:TST=>'B'}]
  end

  def test_tmpl
    _test_tmpl('test_esc1', @varset1, @varset2)
    _test_tmpl('test_esc2', @varset1, @varset2)
    _test_tmpl('test_esc3', @varset1, @varset2)
    _test_tmpl('test_esc4', @varset1, @varset2)
    _test_tmpl('test_var1', @varset1)
    _test_tmpl('test_var2', @varset1)
    _test_tmpl('test_var3', @varset1, @varset2)
    _test_tmpl('test_if1',  @varset1)
    _test_tmpl('test_if2',  @varset1)
    _test_tmpl('test_if3',  @refset1)
    _test_tmpl('test_if4',  @varset1)
    _test_tmpl('test_if5',  @varset1)
    _test_tmpl('test_if7',  @varset1)
    _test_tmpl('test_include1', @varset1)
    _test_tmpl('test_include2', @varset1)
    _test_tmpl('test_include3', @varset1)
    _test_tmpl('test_loop1', @varset1, @refset1)
    _test_tmpl('test_loop2', @varset1, @refset1)
    _test_tmpl('test_loop3', @varset1, @refset1)
    _test_tmpl('test_loop4', @varset1, @refset1)
    _test_tmpl('test_loop5', @varset1, @refset1)
    _test_tmpl_options('test_loop6', {
                         :loop_context_vars=>1,
                         :debug=>1,
                         :global_vars=>1,
                         :die_on_bad_params=>0
                       }, :INNER => @inner, :OUTER => @outer)
    assert true
  end

  private

  def _test_tmpl(file, *args)
    params = args.inject({}) do |result, item|
      result.update(item)
    end
    _test_tmpl_options(file, {:loop_context_vars => 1, :case_sensitive => 0 }, params)
  end

  def _test_tmpl_options(file, option, params)
    Dir.chdir 'templates-Pro'
    tmpl = HTML::Template::Pro.new(option.merge({:filename => file + '.tmpl'}))
    tmpl.param(params)
    dryrun(tmpl, file)
    Dir.chdir '..'
  end

  def dryrun(tmpl, file)
    File.open("#{file}.raw", 'w') do |f|
      tmpl.output(:print_to => f)
    end
    assert_equal File.read("#{file}.out"), File.read("#{file}.raw")
    File.unlink("#{file}.raw")
    output = tmpl.output;
    assert_equal File.read("#{file}.out"), output
  end
end

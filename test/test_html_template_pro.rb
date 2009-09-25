#!/usr/bin/env ruby
# -*- coding: iso-8859-1 -*-

require 'test/unit'
require 'html/template/pro'

class TestHtmlTemplatePro < Test::Unit::TestCase

  @@varset1 = {
    :VAR1 => 'VAR1', :VAR2  => 'VAR2',
    :VAR3 => 'VAR3', :VAR10 => 'VAR10',
  }

  @@varset2 = {
    :STUFF1 => '\<>"; %FA' + "hidden:\r\012end",
    :STUFF2 => 'Some"' + "' Txt'",
  }

  @@refset1 = {
    :HASHREF0 => [],
    :HASHREF2 => [{}, {}],
    :HASHREF1 => [
                  { :LOOPVAR1=>'LOOP1-VAR1',:LOOPVAR2=>'LOOP1-VAR2',
                    :LOOPVAR3=>'LOOP1-VAR3',:LOOPVAR10=>'LOOP1-VAR10'},
                  { :LOOPVAR1=>'LOOP2-VAR1',:LOOPVAR2=>'LOOP2-VAR2',
                    :LOOPVAR3=>'LOOP2-VAR3',:LOOPVAR10=>'LOOP2-VAR10'},
                  { :LOOPVAR1=>'LOOP3-VAR1',:LOOPVAR2=>'LOOP3-VAR2',
                    :LOOPVAR3=>'LOOP3-VAR3',:LOOPVAR10=>'LOOP3-VAR10'},
                  { :LOOPVAR1=>'LOOP4-VAR1',:LOOPVAR2=>'LOOP4-VAR2',
                    :LOOPVAR3=>'LOOP4-VAR3',:LOOPVAR10=>'LOOP4-VAR10'},
                 ],
  }

  @@outer = [{:TEST=>'1'},{:TEST=>'2'},{:TEST=>'3'}]
  @@inner = [{:TST=>'A'},{:TST=>'B'}]

  @@refset2 = { :INNER => @@inner, :OUTER => @@outer }

  def test_tmplpro
    test_tmpl_std('test_esc1', @@varset1, @@varset2)
    test_tmpl_std('test_esc2', @@varset1, @@varset2)
    test_tmpl_std('test_esc3', @@varset1, @@varset2)
    test_tmpl_std('test_esc4', @@varset1, @@varset2)
    test_tmpl_std('test_var1', @@varset1)
    test_tmpl_std('test_var2', @@varset1)
    test_tmpl_std('test_var3', @@varset1, @@varset2)
    test_tmpl_std('test_if1',  @@varset1)
    test_tmpl_std('test_if2',  @@varset1)
    test_tmpl_std('test_if3',  @@refset1)
    test_tmpl_std('test_if4',  @@varset1)
    test_tmpl_std('test_if5',  @@varset1)
    test_tmpl_std('test_if7',  @@varset1)
    test_tmpl_std('test_include1', @@varset1)
    test_tmpl('test_include2', { :max_include => 10 }, @@varset1)
    test_tmpl_std('test_include3', @@varset1)
    test_tmpl('test_include4', { :path => ['include/1', 'include/2'] });
    test_tmpl('test_include5', {
                :path => ['include/1', 'include/2'],
                :search_path_on_include => 1,
              });
    test_tmpl_std('test_loop1', @@varset1, @@refset1)
    test_tmpl_std('test_loop2', @@varset1, @@refset1)
    test_tmpl_std('test_loop3', @@varset1, @@refset1)
    test_tmpl_std('test_loop4', @@varset1, @@refset1)
    test_tmpl_std('test_loop5', @@varset1, @@refset1)
    test_tmpl('test_loop6', {
                         :loop_context_vars => true,
                         :global_vars => true,
              }, @@refset2)

    test_tmpl_std('include/2', { :LIST => [{ :TEST => 1 }, { :TEST => 2 }] });
    test_tmpl('test_broken1', { :debug => -1 }, @@varset1, @@refset1);

    assert true
  end

  private

  def test_tmpl(file, option, *args)
    params = args.inject({}) do |result, item|
      result.update(item)
    end
    Dir.chdir 'templates-Pro'
    tmpl = HTML::Template::Pro.new({ :filename => file + '.tmpl',
                                     :debug => $DEBUG || 0 }.merge(option) )
    tmpl.param(params)
    dryrun(tmpl, file);
    Dir.chdir '..'
  end

  @@case_ext = {
    :loop_context_vars => true,
    :case_sensitive => false,
  }

  @@case_int = {
    :loop_context_vars => true,
    :case_sensitive => true,
    :tmpl_var_case => HTML::Template::Pro::ASK_NAME_UPPERCASE,
  }

  def test_tmpl_std(file, *args)
    test_tmpl(file, @@case_ext, *args)
    test_tmpl(file, @@case_int, *args)
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

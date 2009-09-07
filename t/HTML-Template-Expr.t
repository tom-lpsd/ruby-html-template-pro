# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl HTML-Template-Pro.t'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test;
BEGIN { plan tests => 1+2*14 };
#use HTML::Template;
use HTML::Template::Pro;
ok(1); # If we made it this far, we're ok.

#########################

my $tmpl;
my $output;

my $DEBUG=$ENV{HTMLTEMPLATEPRODEBUG};
$DEBUG||=0;

HTML::Template::Pro->register_function('registered_func'=>sub { return shift(); });
HTML::Template::Pro->register_function('hello_string'=>sub { return 'hello!'; });
HTML::Template::Pro->register_function('arglist'=>sub { return '['.join('][',@_).']'; });
HTML::Template::Pro->register_function( f1 => sub { return "F1: @_"; });
HTML::Template::Pro->register_function( f2 => sub { return "F2: @_"; });
HTML::Template::Pro->register_function( fUNDEF => sub { return undef; });

my @exprset1=(ONE=>1,TWO=>2,THREE=>3,ZERO=>0,MINUSTEN=>-10, FILE=>'test_if1.tmpl', TWENTY=>20,FOURTY=>50);
my @brunoext=('foo.bar'=>'<test passed>');
my @refset1=(
HASHREF0=>[],
HASHREF2=>[{},{}],
HASHREF1=>[
{LOOPVAR1=>'LOOP1-VAR1',LOOPVAR2=>'LOOP1-VAR2',LOOPVAR3=>'LOOP1-VAR3',LOOPVAR10=>'LOOP1-VAR10'},
{LOOPVAR1=>'LOOP2-VAR1',LOOPVAR2=>'LOOP2-VAR2',LOOPVAR3=>'LOOP2-VAR3',LOOPVAR10=>'LOOP2-VAR10'},
{LOOPVAR1=>'LOOP3-VAR1',LOOPVAR2=>'LOOP3-VAR2',LOOPVAR3=>'LOOP3-VAR3',LOOPVAR10=>'LOOP3-VAR10'},
{LOOPVAR1=>'LOOP4-VAR1',LOOPVAR2=>'LOOP4-VAR2',LOOPVAR3=>'LOOP4-VAR3',LOOPVAR10=>'LOOP4-VAR10'},
]);


test_tmpl('test_expr1', @exprset1);
test_tmpl('test_expr2', @exprset1);
test_tmpl('test_expr3', @exprset1);
test_tmpl('test_expr4', @brunoext);
test_tmpl('test_expr5', @exprset1);
test_tmpl('test_expr6', @exprset1);
test_tmpl('test_expr7', @refset1);
test_tmpl('test_expr8', @exprset1);
test_tmpl('test_expr9', @exprset1);
test_tmpl('test_expr10', @exprset1);
test_tmpl('test_expr11', @exprset1);
test_tmpl('test_expr12', @exprset1);
test_tmpl('test_expr13', @exprset1);
test_tmpl('test_expr14', @exprset1);

# -------------------------

sub test_tmpl {
    my $testname=shift;
    my $tmpl;
    print "\n--------------- Test: $testname ---------------------\n";
    chdir 'templates-Pro';
    my $file=$testname;
    $tmpl=HTML::Template::Pro->new(filename=>$file.'.tmpl', loop_context_vars=>1, case_sensitive=>0,debug=>$DEBUG, functions=>{'hello' => sub { return "hi, $_[0]!" }});
    $tmpl->param(@_);
    # per-object extencion
    $tmpl->register_function('per_object_call' => sub { return shift()."-arg"});
    $tmpl->register_function('perobjectcall2' => sub { return shift()."-arg"});
    &dryrun($tmpl,$file);
    chdir '..';
}

sub dryrun {
    my $tmpl=shift;
    my $file=shift;
    open (OUTFILE, ">$file.raw") || die "can't open $file.raw: $!";
    binmode (OUTFILE);
    $tmpl->output(print_to => *OUTFILE);
    close (OUTFILE) || die "can't close $file.raw: $!";
    my $files_equal=&catfile("$file.raw") eq &catfile("$file.out");
    if ($files_equal) {
	ok($files_equal) && unlink "$file.raw";
    } else {
	if ($^O eq 'MSWin32') {
		print STDERR "\n", `fc $file.out $file.raw`;
	}
	elsif (-x '/usr/bin/diff') {
	    print STDERR `diff -C 3 $file.out $file.raw`;
	} else {
	    print STDERR "# >>> ---$file.raw---\n$output\n>>> ---end $file.raw---\n";
	}
    }
    my $output=$tmpl->output();
    ok (defined $output and $output eq &catfile("$file.out"));
}

sub catfile {
    my $file=shift;
    open (INFILE, $file) || die "can't open $file: $!";
    binmode (INFILE);
    local $/;
    my $catfile=<INFILE>;
    close (INFILE) || die "can't close $file: $!";
    return $catfile;
}


### Local Variables: 
### mode: perl
### End: 

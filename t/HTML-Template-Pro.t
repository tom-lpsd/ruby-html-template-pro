# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl HTML-Template-Pro.t'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test;
BEGIN {plan tests => 1+2*(19+4+1) };
use File::Spec;
#use HTML::Template;
use HTML::Template::Pro;
ok(1); # If we made it this far, we're ok.

#########################

my $tmpl;
my $output;

my $DEBUG=$ENV{HTMLTEMPLATEPRODEBUG};
$DEBUG||=0;

my @varset1=(VAR1=>VAR1,VAR2=>VAR2,VAR3=>VAR3,VAR10=>VAR10);
my @varset2=(STUFF1 => '\<>"; %FA'."hidden:\r\012end", STUFF2=>'Срср пур');
my @refset1=(
HASHREF0=>[],
HASHREF2=>[{},{}],
HASHREF1=>[
{LOOPVAR1=>'LOOP1-VAR1',LOOPVAR2=>'LOOP1-VAR2',LOOPVAR3=>'LOOP1-VAR3',LOOPVAR10=>'LOOP1-VAR10'},
{LOOPVAR1=>'LOOP2-VAR1',LOOPVAR2=>'LOOP2-VAR2',LOOPVAR3=>'LOOP2-VAR3',LOOPVAR10=>'LOOP2-VAR10'},
{LOOPVAR1=>'LOOP3-VAR1',LOOPVAR2=>'LOOP3-VAR2',LOOPVAR3=>'LOOP3-VAR3',LOOPVAR10=>'LOOP3-VAR10'},
{LOOPVAR1=>'LOOP4-VAR1',LOOPVAR2=>'LOOP4-VAR2',LOOPVAR3=>'LOOP4-VAR3',LOOPVAR10=>'LOOP4-VAR10'},
]);
my @outer=({TEST=>'1'},{TEST=>'2'},{TEST=>'3'});
my @inner=({TST=>'A'},{TST=>'B'});

if ($ENV{HTMLTEMPLATEPROBROKEN}) {
    # manual test
    test_tmpl('test_broken', @varset1, @refset1);
}

test_tmpl('test_esc1', @varset1, @varset2);
test_tmpl('test_esc2', @varset1, @varset2);
test_tmpl('test_esc3', @varset1, @varset2);
test_tmpl('test_esc4', @varset1, @varset2);

test_tmpl('test_var1', @varset1);
test_tmpl('test_var2', @varset1);
test_tmpl('test_var3', @varset1, @varset2);
test_tmpl('test_if1',  @varset1);
test_tmpl('test_if2',  @varset1);
test_tmpl('test_if3',  @refset1);
test_tmpl('test_if4',  @varset1);
test_tmpl('test_if5',  @varset1);
test_tmpl('test_if7',  @varset1);
test_tmpl('test_include1', @varset1);
test_tmpl('test_include2', @varset1);
test_tmpl('test_include3', @varset1);
test_tmpl('test_loop1', @varset1, @refset1);
test_tmpl('test_loop2', @varset1, @refset1);
test_tmpl('test_loop3', @varset1, @refset1);
test_tmpl('test_loop4', @varset1, @refset1);
test_tmpl('test_loop5', @varset1, @refset1);
test_tmpl_options('test_loop6',[loop_context_vars=>1,debug=>1,global_vars=>1,die_on_bad_params=>0], INNER=>\@inner, OUTER=>\@outer);

# todo: use config.h and grep defines from here
# if IMITATE==1 (-DCOMPAT_ALLOW_NAME_IN_CLOSING_TAG)
#test_tmpl('test_if6',  @varset1);
#


test_tmpl('include/2', 'list', [{test => 1}, {test=>2}]);


my $devnull=File::Spec->devnull();
if (defined $devnull) {
    close (STDERR);
    #open(STDERR, '>>', $devnull); # is better, but seems not for perl 5.005
    open (STDERR, '>/dev/null');
}
test_tmpl('test_broken1', @varset1, @refset1);
# not a test -- to see warnings on broken tmpl
# test_tmpl('test_broken', @varset1, @refset1);


# -------------------------


sub test_tmpl_options {
    my $file=shift;
    my $optref=shift;
    my $tmpl;
    print "\n--------------- Test: $file ---------------------\n";
    chdir 'templates-Pro';
#    $tmpl=HTML::Template->new(filename=>$file.'.tmpl', die_on_bad_params=>0, strict=>0);
    $tmpl=HTML::Template::Pro->new(filename=>$file.'.tmpl', @$optref,debug=>$DEBUG);
    $tmpl->param(@_);
    &dryrun($tmpl,$file);
    chdir '..';
}

sub test_tmpl {
    my ($file,@args)=@_;
    &test_tmpl_options($file, [
			   loop_context_vars=>1, 
			   case_sensitive=>0,
			   debug=>$DEBUG
		       ],@args);
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
	if (-x '/usr/bin/diff') {
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

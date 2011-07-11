################################################################################
#    Copyright (C) 2011 HPCC Systems.
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Affero General Public License as
#    published by the Free Software Foundation, either version 3 of the
#    License, or (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Affero General Public License for more details.
#
#    You should have received a copy of the GNU Affero General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
################################################################################

package Regress::RoxieConfig;

=pod

=head1 NAME
    
Regress::RoxieConfig - perl module used by runregress to execute roxieconfig

=head1 SYNOPSIS

my $submit = Regress::RoxieConfig->new($opts);

$submit->submit($run);

=cut
    
use strict;
use warnings;
use File::Spec::Functions qw(splitpath);
use Exporter;
our @ISA = qw(Exporter);

=pod

=head1 DESCRIPTION

=over

=cut

# PUBLIC

=pod

=item my $submit = Regress::RoxieConfig->new(%options);

Takes a hash of options obtained from a L<Regress::Engine> object. Returns an object used to execute roxieconfig.

=cut

sub new($$)
{
    my ($class, $engine) = @_;
    my $self = {engine => $engine,
                $engine->options()};
    return bless($self, $class);
}

=pod

=item $submit->submit($run, $seq);

Runs roxieconfig to submit a query. Takes a hash as passed to L<Regress::Engine::note_to_run>, and a sequence number. Calls L<Regress::Engine::note_done_run> when completed, passing the run hash and the WUID if known.

=cut

sub submit($$)
{
    my ($self, $run, $seq) = @_;
    my $tmpout = "$run->{outpath}.tmp";
    $self->{seqpaths}->{$seq} = {outpath => $run->{outpath}, tmppath => $tmpout, postfilter => $run->{postfilter}};

    my @commands;
    if($self->{deploy_roxie_queries} eq 'run')
    {
        my %vals = (name => $run->{path},
                    user => $self->{owner},
                    password => $self->{password},
                    roxieconfigaddress => $self->{roxieconfig},
                    makeActive => 1,
                    -noCache => 1,
                    ImportAllModules => 0,
                    ImportImplicitModules => 0,
                    logAction => 0);
        my $args = ["RunECLFile", map("$_=$vals{$_}", sort(keys(%vals)))];
        push(@commands, {command => $self->{roxieconfigcmd} || $self->{engine}->executable_name('roxieconfig'),
                         args => $args,
                         output => $tmpout,
                         done_callback => sub { $self->_on_complete(@_); },
                         seq => $seq});
    }
    else
    {
        if($self->{deploy_roxie_queries} eq 'yes')
        {
            my %vals = (name => $run->{path},
                        user => $self->{owner},
                        password => $self->{password},
                        roxieconfigaddress => $self->{roxieconfig},
                        -noCache => 1,
                        ImportAllModules => 0,
                        ImportImplicitModules => 0,
                        makeActive => 1);
            my $args = ["DeployECLFile", map("$_=$vals{$_}", sort(keys(%vals)))];
            push(@commands, {command => $self->{roxieconfigcmd} || $self->{engine}->executable_name('roxieconfig'),
                             args => $args,
                             output => sub { $self->_scan_for_wuid($seq, @_); },
                             done_callback => sub { $self->_report_deployed(@_); },
                             seq => $seq});
        }
        my (undef, undef, $basename) = splitpath($run->{path});
        $basename =~ s/\.ecl//i;
        my $args;
        if (-e $run->{queryxmlpath})
        {
            $args = [$self->{roxieserver}, "-qname", "$basename", "-f", "$run->{queryxmlpath}" ];
        } else {
            $args = [$self->{roxieserver}, "<$basename/>"];
        }
        push(@commands, {command => $self->{testsocketcmd} || $self->{engine}->executable_name('testsocket'),
                         args => $args,
                         output => $tmpout,
                         done_callback => sub { $self->_on_complete(@_); },
                         seq => $seq});
    }
    $self->{engine}->execute(@commands);
}

#PRIVATE

sub _report_deployed($$$$)
{
    my ($self, $seq, $termerror, $termbad) = @_;
    if($termbad)
    {
        my $paths = $self->{seqpaths}->{$seq};
        $self->{engine}->record_error($paths->{tmppath}, $termerror);
        $self->{engine}->normal_copy($paths->{tmppath}, $paths->{outpath}, $paths->{postfilter});
    }
    if($termerror)
    {
        $self->{engine}->log_write("Deployment failed #$seq ($termerror)");
        my $deployout = $self->{deployout}->{$seq};
        if($deployout)
        {
            $self->{engine}->log_write("roxieconfig output was: [" . join("\n", @{$deployout}) . "]");
        }
        else
        {
            $self->{engine}->log_write("roxieconfig produced no output");
        }
        $self->{engine}->note_done_run($seq);
        #MORE: need to decide what to do here: as written, will get missing output; could write roxieconfig output, but would not match in case where failure expected (if there are any)
    }
    else
    {
        $self->{engine}->log_write("Deployed #$seq");
    }
}

sub _on_complete($$$$)
{
    my ($self, $seq, $termerror, $termbad) = @_;
    my $paths = $self->{seqpaths}->{$seq};
    $self->{engine}->record_error($paths->{tmppath}, $termerror) if($termbad);
    $self->{engine}->warning("No output file created by #$seq") unless(-e $paths->{tmppath});
    $self->{engine}->normal_copy($paths->{tmppath}, $paths->{outpath}, $paths->{postfilter});
    unlink($paths->{tmppath});
    $self->{engine}->note_done_run($seq, $self->{wuids}->{$seq});
    $self->{engine}->log_write("Done #$seq (" . ($termerror ? $termerror : "OK") . ")");
}

sub _scan_for_wuid($$$)
{
    my ($self, $seq, $line) = @_;
    $line =~ s/[\r\n]+$//;
    $self->{engine}->log_write("roxieconfig output: [$line]", !$self->{engine}->{verbose});
    push(@{$self->{deployout}->{$seq}}, $line);
    return if($self->{wuids}->{$seq});
    return unless($line =~ /(W[\d-]+)/);
    $self->{wuids}->{$seq} = $1;
    $self->{engine}->log_write("Created $self->{wuids}->{$seq} for #$seq");
}

=pod

=back

=cut

1;


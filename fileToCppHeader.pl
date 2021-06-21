#!/usr/bin/env perl

use strict;

my $filename = shift || die "need filename";
my $accessor = shift || die "need accessor";



my $file = slurp_file($filename);

my $len = length($file);

$file =~ s/\\/\\x5C/g;
$file =~ s/([^\x20-\x7F])/sprintf "\\x%02x", ord($1)/eg;
$file =~ s/"/\\"/g;


print <<END;
#pragma once

static const char *${accessor}__data = "$file";
static const size_t ${accessor}__size = $len;

static std::string ${accessor}() {
    return std::string(${accessor}__data, ${accessor}__size);
}
END




sub slurp_file {
    my $filename = shift // die "need filename";

    open(my $fh, '<', $filename) || die "couldn't open '$filename' for reading: $!";

    local $/;
    return <$fh>;
}

sub unslurp_file {
    my $contents = shift;
    my $filename = shift;

    open(my $fh, '>', $filename) || die "couldn't open '$filename' for writing: $!";

    print $fh $contents;
}

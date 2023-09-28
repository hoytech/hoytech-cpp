#!/usr/bin/env perl

use strict;
use Digest::SHA;

my $dir = shift || die "need directory";
my $accessor = shift || die "need accessor";


print <<END;
#pragma once

#include <string_view>

END

foreach my $path (glob "$dir/*") {
    $path =~ m{([^/]+)$} || die "bad filename: $path";
    my $filename = $1;

    $filename =~ s/\W/_/g;

    my $file = slurp_file($path);
    my $hash = unpack("H*", Digest::SHA::sha256($file));

    my $len = length($file);

    $file =~ s/(.)/sprintf "\\x%02x", ord($1)/seg;

print <<END;

static const char *${accessor}__${filename}__data = "$file";
static const size_t ${accessor}__${filename}__size = $len;

[[maybe_unused]] static std::string_view ${accessor}__${filename}() {
    return std::string_view(${accessor}__${filename}__data, ${accessor}__${filename}__size);
}

[[maybe_unused]] static std::string_view ${accessor}__${filename}__hash() {
    return std::string_view("$hash", 64);
}

END
}






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

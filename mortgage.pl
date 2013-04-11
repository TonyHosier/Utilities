#!c:/perl/bin

##################################################################
$outFile = "mortgage.txt";

##################################################################

sub CalcMonthlyPayment
{
    my $loanAmount = $_[0];
    my $loanLength = $_[1];
    my $interestRate = $_[2];


    my $totalMonths = $loanLength * 12;

    # Figure out the upper and lower bounds on the monthly payment
    # upper bound is the total of principle of loan paid equally over the loan length
    # plus the total interest on the whole loan amount.

    my $upperMonthlyPayment = $loanAmount / $totalMonths;
    $upperMonthlyPayment += ($loanAmount * $interestRate) / 1200;

    # use a lower bound of half the upper bound. Lower bound will never be 0
    my $lowerMonthlyPayment = $upperMonthlyPayment / 2;

    # Use devide-and-conquer to figure out the best monthly payment
	my $months = 0;
	my $principleLeft = $loanAmount;
	my $monthlyPayment;

	do
	{
		# try a payment half way between the uper and lower payment
		$monthlyPayment = ($upperMonthlyPayment + $lowerMonthlyPayment) / 2;

		while ($principleLeft > 0)
		{
			my $interestThisMonth = ($principleLeft * $interestRate) / 1200;
			$principleLeft -= $monthlyPayment - $interestThisMonth;
			$months++;
		}

		if ($months > $totalMonths)		# payment too small - decrease lower bound
		{
			$lowerMonthlyPayment = $monthlyPayment;
		}
		else
		{
			$upperMonthlyPayment = $monthlyPayment;
		}

		# set up for next time and when the loop exits
		$months = 0;
		$principleLeft = $loanAmount;
	}
	while (($upperMonthlyPayment - $lowerMonthlyPayment) > 0.0000005);

	# print out the monthly results
	my $string;
	$string = sprintf("A monthly payment of \$%.2f\n",$monthlyPayment);
	print $string;
	print OUTFILE $string;
	my $interestThisYear = 0.0;
	my $years;
	while ($principleLeft > 0)
	{
		my $interestThisMonth = ($principleLeft * $interestRate) / 1200;
		$interestThisYear += $interestThisMonth;
		$principleLeft -= $monthlyPayment - $interestThisMonth;
		if ($principleLeft < 0)
		{
			$principleLeft = 0;
		}
		$months++;		
		$years = $months / 12;
		if (($months % 12) == 0)
		{
			$string = sprintf "Principle after %2d years is \$%.2f, interest paid this year is \$%.2f\n", $years, $principleLeft, $interestThisYear;
			print $string;
			print OUTFILE $string;
			$interestThisYear = 0;
		}
	}

	# display the total amount paid (interest and capital)
        $string = sprintf ("\nTotal amount paid over loan period is \$%.2f\n", $monthlyPayment * $totalMonths);
	print $string;
	print OUTFILE $string;
}

sub main
{
    my $loanAmount = 100;
    my $loanLength = 15;
    my $interestRate = 3.0;
    print "Enter Loan amount .. \$";
    $loanAmount = <STDIN>;
    print "Enter loan Length (years) .. ";
    $loanLength = <STDIN>;
    print "Enter Interest Rate .. %";
    $interestRate = <STDIN>;

    #open the target file

    if (!open(OUTFILE, ">$outFile"))
    {
        die("Can't open output file $outFile\n");
    }

    # file is open. Now write the data

    print OUTFILE "Mortgage Payment\n";
    print OUTFILE "================\n";
    print OUTFILE "\n";
    print OUTFILE "Loan Amount .. \$".$loanAmount;
    print OUTFILE "Loan Length (years) .. %".$loanLength;
    print OUTFILE "Interest Rate .. %".$interestRate."\n";

    CalcMonthlyPayment($loanAmount, $loanLength, $interestRate);

    close(OUTFILE);
}

##################################################################

main()

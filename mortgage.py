#############################################################################
# mortgage amortization program in python. Given a loan amount, interest rate
# and loan length, calculate the monthly payment needed to pay off the
# mortgage in full
def CalcMonthlyPayment(f, loanAmount, loanLength, interestRate):
    totalMonths = loanLength * 12;

    # Figure out the upper and lower bounds on the monthly payment
    # upper bound is the total of principle of loan paid equally over the loan
    # length plus the total interest on the whole loan amount.

    upperMonthlyPayment = loanAmount / totalMonths;
    upperMonthlyPayment += (loanAmount * interestRate) / 1200;

    # use a lower bound of half the upper bound. Lower bound will never be 0
    lowerMonthlyPayment = upperMonthlyPayment / 2;

    # Use devide-and-conquer (binary search) to figure out the best monthly
    # payment
    months = 0;
    principleLeft = loanAmount;

    while (True):
        # try a payment half way between the uper and lower payment
        monthlyPayment = (upperMonthlyPayment + lowerMonthlyPayment) / 2;

	while (principleLeft > 0) :
	    interestThisMonth = (principleLeft * interestRate) / 1200;
	    principleLeft -= monthlyPayment - interestThisMonth;
	    months = months + 1;

        if (months > totalMonths) :		# payment too small - decrease lower bound
	    lowerMonthlyPayment = monthlyPayment;
        else :
	    upperMonthlyPayment = monthlyPayment;

	# set up for next time and when the loop exits
	months = 0;
	principleLeft = loanAmount;
	if ((upperMonthlyPayment - lowerMonthlyPayment) < 0.0000005):
            break;

    # print out the monthly results
    string = "A monthly payment of ${0:.2f}".format(monthlyPayment);
    print string;
    f.write(string+"\n");
    interestThisYear = 0.0;
    while (principleLeft > 0) :
	interestThisMonth = (principleLeft * interestRate) / 1200;
	interestThisYear += interestThisMonth;
	principleLeft -= monthlyPayment - interestThisMonth;
	if (principleLeft < 0) :
            principleLeft = 0;

	months = months + 1;		
        years = months / 12;
	if ((months % 12) == 0) :
            string = "Principle after {0:2d} years is ${1:.2f}, interest paid this year is ${2:.2f}".format(years, principleLeft, interestThisYear);
            print string;
	    f.write(string+"\n");
	    interestThisYear = 0;

    # display the total amount paid (interest and capital)
    string = "\nTotal amount paid over loan period is ${0:.2f}".format(float(monthlyPayment*totalMonths));
    print string;
    f.write(string+"\n");

##################################################################
# Main routine.
def main():
    # Ask user for input, open the target file and write the title info
    loanAmount = float(raw_input("Enter Loan amount .. $"));
    loanLength = float(raw_input("Enter loan Length (years) .. "));
    interestRate = float(raw_input("Enter Interest Rate .. %"));

    f = open("mortgage.txt", 'w');

    # file is open. Now write the data

    f.write("Mortgage Payment\n");
    f.write("================\n");
    f.write("\n");
    f.write("Loan Amount .. $%d\n" % loanAmount);
    f.write("Loan Length (years) .. %d\n" % loanLength);
    f.write("Interest Rate .. %f\n\n" % interestRate);
    print;

    CalcMonthlyPayment(f, loanAmount, loanLength, interestRate);

    f.close();

##################################################################

main();

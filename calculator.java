// This is a simplified version of a calculator program. No handling of edge cases or safe operation checks or whatsoever.

public class Calculator {
  // constructor
  public Calculator() {

  }
  
  // add method
  public int add(int a, int b) {
    return a + b;
  }

  // subtract method
  public int subtract(int a, int b) {
    return a - b;
  }

  // multiply method
  public int multiply(int a, int b) {
    return a * b;
  }

  // divide method
  public int divide(int a, int b) {
    return a / b;
  }

  // modulo method
   public int modulo(int a, int b) {
    return a % b;
   }

  // main method
  public static void main(String[] args) {
    Calculator myCalculator = new Calculator();
    System.out.println(myCalculator.add(5, 7));
    System.out.println(myCalculator.subtract(45, 11));
  }
  
}
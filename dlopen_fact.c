/* Pr�klad knihovny pro program dlopen: funkce factorial() */

int factorial(int n)
{
  int i,f=1;
  for(i=2;i<=n;i++)
    f*=i;
  return f;
}

public class Main
{
  public static void main (String[] args)
  {
    for (int j = 0; j < 100; j++)
            {
                ImmutableText myText = ImmutableText.valueOf("hello");
                for (int i = 0; i < 100000; i++)
                {
                    myText = myText.insert(i, "1");
                }

                for (int i = 0; i < 100000; i++)
                {
                    myText = myText.delete(0, 1);
                }
            }
  }
}

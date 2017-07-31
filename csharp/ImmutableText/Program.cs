using System;

namespace ImmutableText
{
    class MainClass
    {
        public static void Main(string[] args)
        {
            for (int j = 0; j < 100; j++)
            {
                var myText = new ImmutableText("hello");
                for (int i = 0; i < 100000; i++)
                {
                    myText = myText.InsertText(i, "1");
                }

                for (int i = 0; i < 100000; i++)
                {
                    myText = myText.RemoveText(0, 1);
                }
            }

        }
    }
}

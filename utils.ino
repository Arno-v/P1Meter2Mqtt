void blinkLed(int numberOfBlinks, int msBetweenBlinks)
{
  for (int i = 0; i < numberOfBlinks; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(msBetweenBlinks);
    digitalWrite(LED_BUILTIN, LOW);
    if (i != numberOfBlinks - 1)
    {
      delay(msBetweenBlinks);
    }
  }
}

#include "RemoteInput.h"
#include <IRremote.hpp>

RemoteInput::RemoteInput(gpio_num_t input_pin, gpio_num_t pwr_pin, gpio_num_t gnd_pin, gpio_num_t led_pin) : m_input_pin(input_pin), m_pwr_pin(pwr_pin), m_gnd_pin(gnd_pin), m_led_pin(led_pin)
{
}

void RemoteInput::start()
{
  // power up the receiver
  if (m_gnd_pin != -1)
  {
    pinMode(m_gnd_pin, OUTPUT);
    digitalWrite(m_gnd_pin, LOW);
  }
  if (m_pwr_pin != -1)
  {
    pinMode(m_pwr_pin, OUTPUT);
    digitalWrite(m_pwr_pin, HIGH);
  }
  // add a pullup to the input pin - TODO - is this needed?
  pinMode(m_input_pin, INPUT_PULLUP);
  // start the receiver
  IrReceiver.begin(m_input_pin, m_led_pin != -1, m_led_pin);
}

void RemoteInput::stop()
{
  // power down the receiver
  if (m_pwr_pin != -1)
  {
    digitalWrite(m_pwr_pin, LOW);
  }
  // stop the receiver
  IrReceiver.end();
}

RemoteCommands RemoteInput::getLatestCommand()
{
  if (IrReceiver.decode())
  {
    RemoteCommands command = static_cast<RemoteCommands>(IrReceiver.decodedIRData.command);
    IrReceiver.resume();
    // is it one of our recognized commands?
    if (command == RemoteCommands::CHANNEL_DOWN || command == RemoteCommands::CHANNEL_UP || command == RemoteCommands::EIGHT || command == RemoteCommands::FIVE || command == RemoteCommands::FOUR || command == RemoteCommands::NINE || command == RemoteCommands::ONE || command == RemoteCommands::SEVEN || command == RemoteCommands::SIX || command == RemoteCommands::THREE || command == RemoteCommands::TWO || command == RemoteCommands::ZERO || command == RemoteCommands::FAST_FORWARD || command == RemoteCommands::PLAY_PAUSE || command == RemoteCommands::POWER || command == RemoteCommands::REWIND || command == RemoteCommands::STOP || command == RemoteCommands::VOLUME_DOWN || command == RemoteCommands::VOLUME_UP)
    {
      return command;
    }
  }
  return RemoteCommands::UNKNOWN;
}

#include <types.h>
#include <multiboot.h>
#include <display/VGABuffer.h>
#include <hw/cpu.h>
#include <hw/SerialPort.h>
#include <memory/PhysicalMemoryManager.h>
#include <lib/String.h>
#include <lib/unique_ptr.h>
#include <debug/Logger.h>
#include <utils/SingletonFactory.h>
#include <utils/Singleton.h>
#include <utils/NonCopyable.h>

using namespace Display;
using namespace Debug;
using namespace Device;
using namespace Utils;
using namespace hw;
using namespace std;

extern "C" void KernelMain(uint32_t magic, Multiboot *multiboot)
{
  unused(magic);
  unused(multiboot);

  auto com1 = std::unique_ptr(new SerialPort(SerialPort::COM1));

  SingletonFactory<Logger, WritableDevice *>().Create(com1.Get());
  SingletonFactory<VGABuffer>().Create();

  VGABuffer::GetInstance()->Clear();
  VGABuffer::GetInstance()->SetColor(VGABuffer::Color::Yellow, VGABuffer::Color::Black);
  VGABuffer::GetInstance()->Write("Welcome to our 64-bit kernel: %d, 0x%x, %s, %c!", 28, 14, "huhu", 'c');

  Logger::GetInstance()->Log("main", "This is a logging test nr. %d", 1);
  Logger::GetInstance()->Log("main", "This is a logging test nr. %d", 2);

  multiboot = PhysicalMemoryManager::GetInstance()->ConvertPhysicalAddressToVirtual(multiboot);

  // if (magic != MultibootMagic)
  // {
  //   // TODO: panic!
  //   VGABuffer::instance()->SetColor(VGABuffer::Color::Red, VGABuffer::Color::Black);
  //   VGABuffer::instance()->Write("Invalid multiboot magic (expected: 0x%x, got 0x%x)!", MultibootMagic, magic);

  //   return;
  // }

  hw::cpu::HangForever();
}

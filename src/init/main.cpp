#include <types.h>
#include <init/multiboot.h>
#include <display/VGABuffer.h>
#include <hw/cpu.h>
#include <hw/SerialPort.h>
#include <memory/PhysicalMemoryManager.h>
#include <lib/String.h>
#include <lib/unique_ptr.h>
#include <debug/Logger.h>
#include <utils/SingletonFactory.h>
#include <utils/Singleton.h>
#include <utils/Bits.h>

using namespace Utils;
using namespace Display;
using namespace Debug;
using namespace Device;
using namespace hw;
using namespace std;

extern "C" void KernelMain(uint64_t magic, Multiboot::Header *multiboot)
{
  auto com1 = std::unique_ptr(new SerialPort(SerialPort::COM1));

  SingletonFactory<Logger, WritableDevice *>().Create(com1.Get());

  Assert(magic == Multiboot::Magic, "Bad magic number");
  Assert(Bits::IsAligned((uintptr_t)multiboot, 8), "Multiboot structure must be 8-bytes aligned");

  SingletonFactory<VGABuffer>().Create();

  VGABuffer::GetInstance()->Clear();

  VGABuffer::GetInstance()->SetColor(VGABuffer::Color::Yellow, VGABuffer::Color::Black);
  VGABuffer::GetInstance()->Write("Welcome to my 64-bit kernel: %d, 0x%x, %s, %c!", 28, 14, "huhu", 'c');

  Logger::GetInstance()->Log("main", String::Format("%d", Bits::AlignUp(28, 8)));

  multiboot = PhysicalMemoryManager::GetInstance()->ConvertPhysicalAddressToVirtual(multiboot);

  cpu::HangForever();
}

#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

ifeq ($(VERBOSE),)
  SILENCE := @
else
  SILENCE :=
endif

# Where and what are we building?
ifeq ($(APPLICATION),)
  APPLICATION := sample/blinky
endif
ifeq ($(TARGET_MCU),)
  TARGET_MCU := attiny85
endif
ifeq ($(CLOCK_FREQUENCY),)
  # Digispark default
  CLOCK_FREQUENCY := 16500000
endif

# On Debian/Ubuntu, sudo apt install gcc-avr avr-libc
CC :=      $(SILENCE)avr-gcc
OBJCOPY := $(SILENCE)avr-objcopy
OBJDUMP := $(SILENCE)avr-objdump
OBJSIZE := $(SILENCE)avr-size
RMDIR :=   $(SILENCE)rm -rf

# List source files here...
C_SRCS :=  $(wildcard $(APPLICATION)/*.c) \
           $(wildcard lib/*.c) \
# keep this comment to consume final backslash

# Derive .o and .d filenames from .c
OUTPUT_DIR := build-$(TARGET_MCU)
OUTPUT_FILE := $(OUTPUT_DIR)/chaserlights.elf
OBJS := $(C_SRCS:%.c=$(OUTPUT_DIR)/%.o)
C_DEPS := $(C_SRCS:%.c=$(OUTPUT_DIR)/%.d)

# Require .HEX output (for CI)
build: $(OUTPUT_FILE:.elf=.hex)

# Require all outputs (for Atmel Studio)
all: $(OUTPUT_FILE:.elf=.hex) $(OUTPUT_FILE:.elf=.eep) $(OUTPUT_FILE:.elf=.lss) $(OUTPUT_FILE:.elf=.srec) doc

clean:
	-$(RMDIR) $(OUTPUT_DIR)

rebuild: clean build

# AVR8/GNU C Compiler

$(OUTPUT_DIR)/%.o: %.c
	@echo $<
	mkdir -p $(dir $@)
	$(CC) -iquote inc -iquote $(APPLICATION) -iquote etc \
          -x c -funsigned-char -funsigned-bitfields \
          -DTARGET_MCU=$(TARGET_MCU) -DTARGET_MCU_IS_$(TARGET_MCU)=1 -DF_CPU=$(CLOCK_FREQUENCY)UL \
          -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -Wall -mmcu=$(TARGET_MCU) \
          -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -Os -g \
          -o "$@" "$<"

# Header dependencies
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

# AVR8/GNU C Linker
$(OUTPUT_FILE): $(OBJS)
	@echo $@
	$(CC) \
          -o$@ $^ \
          -Wl,--script=etc/linker.ld \
          -Wl,-Map=$(@:.elf=.map) -Wl,--gc-sections \
          -mmcu=$(TARGET_MCU)
	$(OBJSIZE) $@

# Output formats
%.hex: %.elf
	$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures $< $@

%.lst: %.elf
	$(OBJDUMP) --disassemble --source --syms $< >$@

# Utility
flash: $(OUTPUT_FILE:.elf=.hex)
	$(SILENCE)../hard/tools/bin/micronucleus --run $<

${UC_INCLUDE}/%.h : ${ROM_SRC_DIR}/%.bin
	dotnet run --project ../romtoc $< ${UC_INCLUDE}
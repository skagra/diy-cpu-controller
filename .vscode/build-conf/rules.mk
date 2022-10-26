%.bin $(BIN)/%.h : %.h
	../dotnet run $< $(@D)/$(*F).h 
# This file is generated by rake. Do not edit.

module Etc
  class Passwd < FFI::Struct
    layout :pw_name, :string, 0,
           :pw_passwd, :string, 4,
           :pw_uid, :uint, 8,
           :pw_gid, :uint, 12,
           :pw_dir, :string, 20,
           :pw_shell, :string, 24








    def name; self[:pw_name]; end
    def passwd; self[:pw_passwd]; end
    def uid; self[:pw_uid]; end
    def gid; self[:pw_gid]; end
    def dir; self[:pw_dir]; end
    def shell; self[:pw_shell]; end
  end

  class Group < FFI::Struct
    layout :gr_name, :string, 0,
           :gr_gid, :uint, 8








    def name; self[:gr_name]; end
    def gid; self[:gr_gid]; end
  end

  def self.getlogin
    getpwuid.name
  end

  def self.getpwnam(name)
    login = StringValue(name)

    passwd_ptr = Platform::POSIX.getpwnam(name)
    if passwd_ptr.nil?
      raise ArgumentError, "cannot find user - #{name}"
    end

    Passwd.new(passwd_ptr)
  end

  def self.getpwuid(uid = Process.uid)
    uid = Type.coerce_to(uid, Integer, :to_int)

    passwd_ptr = Platform::POSIX.getpwuid(uid)
    if passwd_ptr.nil?
      raise ArgumentError, "cannot find user - #{uid}"
    end

    Passwd.new(passwd_ptr)
  end

  def self.setpwent
    Platform::POSIX.setpwent
  end

  def self.getpwent
    passwd_ptr = Platform::POSIX.getpwent
    return nil if passwd_ptr.nil?

    Passwd.new(passwd_ptr)
  end

  def self.endpwent
    Platform::POSIX.endpwent
  end

  def self.passwd
    setpwent

    loop do
      pw = getpwent
      return if pw.nil?

      yield pw
    end
  ensure
    endpwent
  end

  def self.getgrnam(name)
    name = StringValue(name)

    group_ptr = Platform::POSIX.getgrnam(name)
    if group_ptr.nil?
      raise ArgumentError, "cannot find group - #{name}"
    end

    Group.new(group_ptr)
  end

  def self.getgrgid(gid = Process.gid)
    gid = Type.coerce_to(gid, Integer, :to_int)

    group_ptr = Platform::POSIX.getgrgid(gid)
    if group_ptr.nil?
      raise ArgumentError, "cannot find group - #{gid}"
    end

    Group.new(group_ptr)
  end

  def self.setgrent
    Platform::POSIX.setgrent
  end

  def self.getgrent
    group_ptr = Platform::POSIX.getgrent
    return nil if group_ptr.nil?

    Group.new(group_ptr)
  end

  def self.endgrent
    Platform::POSIX.endgrent
  end

  def self.group
    setgrent

    loop do
      gr = getgrent
      return if gr.nil?

      yield gr
    end
  ensure
    endgrent
  end
end
